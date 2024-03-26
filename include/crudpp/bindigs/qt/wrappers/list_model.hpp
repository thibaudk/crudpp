#pragma once

#include <QAbstractListModel>
#include <QJsonDocument>
#include <wobjectdefs.h>

#include <crudpp/bindigs/qt/interface/net_manager.hpp>
#include <crudpp/bindigs/qt/interface/bridge.hpp>
#include "list.hpp"

namespace qt
{
template <typename T, bool has_primary_key = false>
class list_model : public QAbstractListModel
{
    W_OBJECT(list_model)

public:
    explicit list_model(QObject* parent = nullptr)
        : QAbstractListModel{parent}
    {
        connect(m_list,
                &list<T>::addWith,
                [this] (const QJsonObject& obj)
                {
                    net_manager::instance().postToKey(T::table(),
                        QJsonDocument{obj}.toJson(),
                        [this] (const QJsonObject& rep)
                        { m_list->append(model<T>{rep}); },
                        "AddWith error");
                });

        connect(m_list,
                &list<T>::get,
                [this] ()
                {
                    net_manager::instance().getFromKey(T::table(),
                                                       [this](const QByteArray& bytes)
                                                       { m_list->read(bytes); });
                });

        beginResetModel();

        connect(m_list, &list<T>::preItemsAppended,
                this, [this](int number)
                {
                    const int index = m_list->size();
                    beginInsertRows(QModelIndex(), index, index + number - 1);
                });

        connect(m_list, &list<T>::postItemsAppended,
                this, [this]()
                { endInsertRows(); });

        connect(m_list, &list<T>::preItemsRemoved,
                this, [this](int first, int last)
                { beginRemoveRows(QModelIndex(), first, last); });

        connect(m_list, &list<T>::preItemsRemoved,
                this, [this](int first, int last)
                { beginRemoveRows(QModelIndex(), first, last); });

        connect(m_list, &list<T>::postItemsRemoved,
                this, [this]()
                { endRemoveRows(); });

        connect(m_list, &list<T>::dataChangedAt,
                this, [this](int row)
                { dataChanged(index(row), index(row)); });

        connect(m_list, &list<T>::save,
                this, [this](int row)
                { setLoading(row, true); });

        connect(m_list, &list<T>::remove,
                this, [this](int row)
                { setLoading(row, true); });

        connect(m_list, &list<T>::loaded,
                this, [this](int row)
                { setLoading(row, false); });

        endResetModel();
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;

        return m_list->size();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant{};

        return m_list->items().at(index.row()).data(role);
    }

    bool setData(const QModelIndex& index,
                 const QVariant& value,
                 int role = Qt::EditRole) override
    {
        m_list->item_at(index.row()).setData(value, role);
        // emit dataChanged for both the curent role and the "flagged_for_update role"
        emit dataChanged(index, index,
                         QVector<int>() << role << model<T>::flagged_for_update_role());
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        return Qt::ItemIsEditable;
    }

    QHash<int, QByteArray> roleNames() const override { return model<T>::roleNames(); }

    list<T>* getList() const { return m_list; }

protected:
    list<T>* m_list{new list<T>};

    void setLoading(int row, bool value)
    {
        int role{model<T>::loading_role()};
        m_list->item_at(row).setData(value, role);
        emit dataChanged(index(row),
                         index(row),
                         QVector<int>() << role << model<T>::flagged_for_update_role());
    }
};

template <typename T>
class list_model<T, true> : public list_model<T, false>
{
public:
    list_model() : list_model<T, false>{}
    {
        QObject::connect(this->m_list,
                         &list<T>::save,
                         [this] (int row)
                         {
                             QJsonObject obj{};
                             auto& item{this->m_list->item_at(row)};
                             item.write(obj);

                             // update if the item was already inserted
                             if (item.inserted())
                             {
                                 // skip if nothing needs updating
                                 // ie. if only the primary key was writen to json
                                 if (obj.size() == 1)
                                 {
                                     emit this->m_list->loaded(row);
                                     return;
                                 }

                                 net_manager::instance().putToKey(make_key(item).c_str(),
                                     QJsonDocument{obj}.toJson(),
                                     [&item, row, obj, this](const QJsonObject& rep)
                                     {
                                         auto objects{bridge::instance().engine
                                                          ->rootObjects()[0]
                                                          ->findChildren<property_holder<T>*>()};

                                         for (auto* p : objects)
                                             if (item.get_aggregate().primary_key.value ==
                                                 p->get_aggregate().primary_key.value)
                                                 p->read(obj);

                                         item.reset_flags();
                                         emit this->m_list->loaded(row);
                                     },
                                     "save error",
                                     [row, this]()
                                     { emit this->m_list->loaded(row); });
                             }
                             else // insert otherwise
                             {
                                 net_manager::instance().postToKey(T::table(),
                                     QJsonDocument{obj}.toJson(),
                                     [&item, row, this](const QJsonObject& rep)
                                     {
                                         item.read(rep);
                                         emit this->m_list->loaded(row);
                                     },
                                     "save error",
                                     [row, this]()
                                     { emit this->m_list->loaded(row); });
                             }
                         });

        QObject::connect(this->m_list,
                         &list<T>::remove,
                         [this] (int row)
                         {
                             auto& item{this->m_list->item_at(row)};

                             // delete on the server if it exists
                             if (item.inserted())
                             {
                                 net_manager::instance().deleteToKey(make_key(item).c_str(),
                                     [this, &item, row](const QJsonValue& rep)
                                     {
                                         auto objects{bridge::instance().engine
                                                          ->rootObjects()[0]
                                                          ->findChildren<property_holder<T>*>()};

                                         for (auto* p : objects)
                                             if (item.get_aggregate().primary_key.value ==
                                                 p->get_aggregate().primary_key.value)
                                                 p->clear();

                                         this->m_list->removeItem(row);
                                         emit this->m_list->loaded(row);
                                     },
                                     "Remove Error",
                                     [this, row] ()
                                     { emit this->m_list->loaded(row); });

                                 return;
                             }

                             // only remove localy otherwise
                             this->m_list->removeItem(row);
                         });
    }

private:
    const std::string make_key(model<T>& item) const
    {
        return make_key(std::move(item.get_aggregate()));
    }

    const std::string make_key(const T&& aggregate) const
    {
        std::string key{T::table()};

        key += '/';
        key += std::to_string(aggregate.primary_key.value);

        return key;
    }
};

} //namespace qt

#include "wobjectimpl.h"
W_OBJECT_IMPL((qt::list_model<T, has_primary_key>), template <typename T, bool has_primary_key>)
