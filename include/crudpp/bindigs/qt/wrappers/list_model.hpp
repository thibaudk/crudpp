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
                                           [this] (const QByteArray& bytes)
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
        auto& item{this->item_at(row)};

        if (!item.flagged_for_update())
            return;

        this->setLoading(row, true);

        QJsonObject obj{};
        item.write(obj);

        // update if the item was already inserted
        if (item.inserted())
        {
            net_manager::instance().putToKey(key(item).c_str(),
                QJsonDocument{obj}.toJson(),
                [&item, row, this](const QJsonObject& rep)
                {
                    const auto id{(item.get_aggregate().*T::primary_key()).value};

                    // FIXME: replace with static vector of pointers to all instances ?
                    auto objects{bridge::instance().engine
                                     ->rootObjects()[0]
                                     ->findChildren<property_holder<T>*>()};

                    for (auto* p : objects)
                        if ((p->get_aggregate().*T::primary_key()).value == id)
                            p->from_item(item);

                    item.reset_flags();
                    this->setLoading(row, false);

                },
                "save error",
                [row, this]()
                { this->setLoading(row, false);
                });
        }
        else // insert otherwise
        {
            net_manager::instance().postToKey(T::table(),
                QJsonDocument{obj}.toJson(),
                [&item, row, this](const QJsonObject& rep)
                {
                    item.read(rep);
                    this->setLoading(row, false);

                },
                "save error",
                [row, this]()
                { this->setLoading(row, false);
                });
        }
    }
    W_INVOKABLE(save)

    void remove(int row)
    {
        this->setLoading(row, true);

        auto& item{this->item_at(row)};

        // delete on the server if it exists
        if (item.inserted())
        {
            net_manager::instance().deleteToKey(key(item).c_str(),
                [this, &item, row](const QJsonValue& rep)
                {
                    const auto id{(item.get_aggregate().*T::primary_key()).value};

                    // FIXME: replace with static vector of pointers to all instances ?
                    auto objects{bridge::instance().engine
                                     ->rootObjects()[0]
                                     ->findChildren<property_holder<T>*>()};

                    for (auto* p : objects)
                        if ((p->get_aggregate().*T::primary_key()).value == id)
                            p->clear();

                    this->removeItem(row);
                    this->setLoading(row, false);

                },
                "Remove Error",
                [this, row] ()
                { this->setLoading(row, false);
                });

            return;
        }

        // only remove localy otherwise
        this->removeItem(row);
    }
    W_INVOKABLE(remove)

    void dataChangedAt (int row) { emit dataChanged(index(row), index(row)); }

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

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        return Qt::ItemIsEditable;
    }

    QHash<int, QByteArray> roleNames() const override { return Type::roleNames(); }

    int size() const { return m_items.size(); }

    QVector<Type> items() const { return m_items; }

    Type& item_at(int index)
    {
        // TODO: implement error handling !!
        return m_items[index];
    }

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

    const std::string key(model<T>& item) const
    {
        return make_key(std::move(item.get_aggregate()));
    }
};
} //namespace qt

#include "wobjectimpl.h"
W_OBJECT_IMPL(qt::list_model<T>, template <typename T>)
