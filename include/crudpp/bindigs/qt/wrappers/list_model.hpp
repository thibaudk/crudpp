#pragma once

#include <QAbstractListModel>
#include <QJsonDocument>
#include <QJsonArray>

#include <wobjectdefs.h>

#include <crudpp/bindigs/qt/interface/net_manager.hpp>
#include <crudpp/bindigs/qt/interface/bridge.hpp>
#include "model.hpp"

namespace qt
{
// TODO: handle lists of objects without primary key
template <typename T>
class list_model : public QAbstractListModel
{
    W_OBJECT(list_model)
    using Type = model<T>;

public:
    explicit list_model(QObject* parent = nullptr)
        : QAbstractListModel{parent}
    {}

    void get()
    {
        net_manager::instance().getFromKey(T::table(),
                                           [this](const QByteArray& bytes)
                                           { read(bytes); });

    }
    W_INVOKABLE(get)

    void addWith(const QJsonObject& obj)
    {
        net_manager::instance().postToKey(T::table(),
            QJsonDocument{obj}.toJson(),
            [this] (const QJsonObject& rep)
            { append(Type{rep}); },
            "AddWith error");
    }
    W_INVOKABLE(addWith)

    void appendItems(int number = 1)
    {
        const int index = size();
        beginInsertRows(QModelIndex(), index, index + number - 1);

        for (int i = 0; i < number; i++)
            m_items.emplace_back(Type{});

        endInsertRows();
    }
    W_INVOKABLE(appendItems)

    void appendItem()
    {
        const int index = size();
        beginInsertRows(QModelIndex(), index, index);

        m_items.emplace_back(Type{});

        endInsertRows();
    }
    W_INVOKABLE(appendItem)

    void append(Type&& item)
    {
        const int index = size();
        beginInsertRows(QModelIndex(), index, index);

        m_items.emplace_back(item);

        endInsertRows();
    }
    // W_INVOKABLE(append)

    void removeItems(int first, int last)
    {
        beginRemoveRows(QModelIndex(), first, last);

        m_items.erase(m_items.begin() + first,
                      std::next(m_items.begin() + last));

        endRemoveRows();
    }
    W_INVOKABLE(removeItems)

    // void removeItems(int number = 1)
    // {
    //     const auto count = size();
    //     removeItems(count - number, count - 1);
    // }
    // W_INVOKABLE(removeItems)

    void removeItem(int index)
    {
        removeItems(index, index);
    }
    W_INVOKABLE(removeItem)

    void clear()
    {
        beginRemoveRows(QModelIndex(), 0, size() - 1);
        m_items.clear();
        endRemoveRows();
    }
    W_INVOKABLE(clear)

    void read(const QJsonArray& array)
    {
        clear();
        beginInsertRows(QModelIndex(), 0, array.size() - 1);

        if (!array.empty())
            for (const auto& json : array)
                m_items.emplace_back(Type{json.toObject()});

        endInsertRows();
    }

    void read(const QJsonObject& obj)
    {
        clear();
        beginInsertRows(QModelIndex(), 0, 1);

        m_items.emplace_back(Type{obj});

        endInsertRows();
    }

    void read(const QByteArray& bytes)
    {
        const auto doc{QJsonDocument::fromJson(bytes)};

        if (doc.isArray())
            read(doc.array());
        else
            read(doc.object());
    }


    void save(int row)
    {
        QJsonObject obj{};
        auto& item{this->item_at(row)};
        item.write(obj);

        // update if the item was already inserted
        if (item.inserted())
        {
            // skip if nothing needs updating
            // ie. if only the primary key was writen to json
            if (obj.size() == 1)
            {
                emit this->loaded(row);
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
                    emit this->loaded(row);
                },
                "save error",
                [row, this]()
                { emit this->loaded(row); });
        }
        else // insert otherwise
        {
            net_manager::instance().postToKey(T::table(),
                QJsonDocument{obj}.toJson(),
                [&item, row, this](const QJsonObject& rep)
                {
                    item.read(rep);
                    emit this->loaded(row);
                },
                "save error",
                [row, this]()
                { emit this->loaded(row); });
        }
    }

    void remove(int row)
    {
        this->setLoading(row, true);

        auto& item{this->item_at(row)};

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

                    this->removeItem(row);
                    emit this->loaded(row);
                },
                "Remove Error",
                [this, row] ()
                { emit this->loaded(row); });

            return;
        }

        // only remove localy otherwise
        this->removeItem(row);
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;

        return size();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant{};

        return m_items[index.row()].data(role);
    }

    bool setData(const QModelIndex& index,
                 const QVariant& value,
                 int role = Qt::EditRole) override
    {
        item_at(index.row()).setData(value, role);
        // emit dataChanged for both the curent role and the "flagged_for_update role"
        emit dataChanged(index,
                         index,
                         QVector<int>() << role << Type::flagged_for_update_role());
        return true;
    }

    void dataChangedAt (int row) { dataChanged(index(row), index(row)); }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        return Qt::ItemIsEditable;
    }

    QHash<int, QByteArray> roleNames() const override { return Type::roleNames(); }

    int size() const { return m_items.size(); }

    QVector<Type> items() const { return m_items; }

    Type& item_at(int index) { return m_items[index]; }

private:
    QVector<Type> m_items{};

    void setLoading(int row, bool value)
    {
        int role{Type::loading_role()};
        item_at(row).setData(value, role);
        emit dataChanged(index(row),
                         index(row),
                         QVector<int>() << role << Type::flagged_for_update_role());
    }

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
W_OBJECT_IMPL(qt::list_model<T>, template <typename T>)
