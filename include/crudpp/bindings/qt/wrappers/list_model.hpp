#pragma once

#include <QAbstractListModel>
#include <QJsonDocument>
#include <QJsonArray>

#include <wobjectdefs.h>

#include <crudpp/bindings/qt/interface/net_manager.hpp>
#include <crudpp/bindings/qt/interface/bridge.hpp>
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
    // VERIFY : should replace by std::deque to avoid realocation problemes ?
    static std::vector<list_model<T>*> instances;

    explicit list_model(QObject* parent = nullptr)
        : QAbstractListModel{parent}
    {
        instances.emplace_back(this);
    }

    ~ list_model()
    {
        instances.erase(std::remove(instances.begin(),
                                    instances.end(),
                                    this),
                        instances.end());
    }

    list_model(const list_model &) = delete;
    void operator = (const list_model &) = delete;

    void get()
    {
        setLoading(true);

        std::string table{T::table()};

        net_manager::instance().getFromKey(table.c_str(),
            [this] (const QByteArray& bytes)
            {
                read(bytes);
                setLoading(false);
            },
            QString::fromStdString(table + " get error"),
            [this] ()
            { setLoading(false); }
            );
    }
    W_INVOKABLE(get)

    void search(const QString& input)
    {
        setLoading(true);

        // QString in{input};
        std::string p{input.toStdString()};
        p.insert(0, "*=");

        // for (const auto& parts : params.split(';', Qt::SkipEmptyParts))
        // {
        //     for (const auto& key_val : parts.split(':', Qt::SkipEmptyParts))
        //         // if ()
        //         // {}
        // }

        std::string table{T::table()};

        net_manager::instance().getFromKey(table.c_str(),
            [this] (const QByteArray& bytes)
            {
                read(bytes);
                setLoading(false);
            },
            QString::fromStdString(table + " search error"),
            [this] ()
            {
                setLoading(false);
            },
            p.c_str());
    }
    W_INVOKABLE(search)

    void addWith(const QJsonObject& obj)
    {
        bridge::instance().increment_load();

        net_manager::instance().postToKey(T::table(),
            QJsonDocument{obj}.toJson(),
            [this] (const QJsonObject& rep)
            {
                append(Type{rep});
                bridge::instance().decrement_load();
            },
            "AddWith error",
            [this] ()
            { bridge::instance().decrement_load(); }
            );
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

    void removeItems(int first, int last)
    {
        beginRemoveRows(QModelIndex(), first, last);

        if (std::next(m_items.begin() + last) > m_items.end())
            m_items.erase(m_items.begin() + first, m_items.end());
        else
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
        beginRemoveRows(QModelIndex(), 0, std::max(0, size() - 1));
        m_items.clear();
        endRemoveRows();
    }
    W_INVOKABLE(clear)

    void read(const QJsonArray& array, bool append = false)
    {
        if (array.empty())
        {
            if (!append) clear();
            return;
        }

        if (append)
        {
            int s{size()};
            beginInsertRows(QModelIndex(), s, s + array.size() - 1);
        }
        else
        {
            clear();
            beginInsertRows(QModelIndex(), 0, array.size() - 1);
        }

        for (const auto& json : array)
            m_items.emplace_back(Type{json.toObject()});

        endInsertRows();
    }

    void read(const QJsonObject& obj, bool append = false)
    {
        if (append)
        {
            int s{size()};
            beginInsertRows(QModelIndex(), s, s);
        }
        else
        {
            clear();
            beginInsertRows(QModelIndex(), 0, 0);
        }

        m_items.emplace_back(Type{obj});

        endInsertRows();
    }

    void read(const QByteArray& bytes, bool append = false)
    {
        const auto doc{QJsonDocument::fromJson(bytes)};

        if (doc.isArray())
            read(doc.array(), append);
        else
            read(doc.object(), append);
    }

    void save(int row)
    {
        auto& item{this->item_at(row)};

        if (!item.flagged_for_update())
            return;

        // update if the item was already inserted
        // insert otherwise
        item.inserted() ? update(item, row) : insert(item, row);
    }
    W_INVOKABLE(save)

    void save_queued(int row)
    {
        auto& item{this->item_at(row)};

        if (!item.flagged_for_update())
        {
            bridge::instance().dequeue();
            return;
        }

        if (item.inserted())
        {
            update(item, true);
            // updates are not considered relevant in the QML queue
            // therefore dequeue is called directly
            // outside of the callback
            bridge::instance().dequeue();
        }
        else
            insert(item, true);
    }
    W_INVOKABLE(save_queued)

    void remove(int row)
    {
        auto& item{this->item_at(row)};

        // delete on the server if it exists
        if (item.inserted())
        {
            del(item, row);
            return;
        }

        // only remove localy otherwise
        this->removeItem(row);
    }
    W_INVOKABLE(remove)

    void remove_queued(int row)
    {
        auto& item{this->item_at(row)};

        // delete on the server if it exists
        if (item.inserted())
        {
            del(item, row, true);
            return;
        }

        // only remove localy otherwise
        this->removeItem(row);
        bridge::instance().dequeue();
    }
    W_INVOKABLE(remove_queued)

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

    void loadingChanged() const
    W_SIGNAL(loadingChanged)

private:
    // list loading
    bool getLoading() { return loading; }
    W_PROPERTY(bool, loading READ getLoading NOTIFY loadingChanged)

    bool loading{false};

    void setLoading(bool value)
    {
        if (value == loading)
            return;

        loading = value;
        emit loadingChanged();
    }

    // item loading
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
        return crudpp::make_key(std::move(item.get_aggregate()));
    }

    void insert(Type& item, bool queued = false)
    {
        QJsonObject obj{};
        item.write(obj);

        bridge::instance().increment_load();

        net_manager::instance().postToKey(T::table(),
            QJsonDocument{obj}.toJson(),
            [&item, this, queued](const QJsonObject& rep)
            {
                item.read(rep);
                bridge::instance().decrement_load();
                if (queued) bridge::instance().dequeue();
            },
            "save error");
    }

    void update(Type& item, int row)
    {
        QJsonObject obj{};
        item.write(obj);

        setLoading(row, true);

        net_manager::instance().putToKey(key(item).c_str(),
            QJsonDocument{obj}.toJson(),
            [&item, row, this] (const QJsonObject& rep)
            {
                using namespace crudpp;

                const auto id{t_trait<T>::pk_value(item.get_aggregate())};

                for (auto* p : property_holder<T>::instances)
                    if (t_trait<T>::pk_value(p->get_aggregate()) == id)
                        p->from_item(item);

                for (auto* m : instances)
                {
                    for (int i{0}; i < m->size(); i++)
                    {
                        auto& other_item{m->item_at(i)};

                        if (t_trait<T>::pk_value(other_item.get_aggregate()) == id)
                        {
                            other_item.set(item.get_aggregate());
                            m->dataChangedAt(i);
                            break;
                        }
                    }
                }

                item.reset_flags();
                setLoading(row, false);
            },
            "save error",
            [row, this]()
            { setLoading(row, false); }
            );
    }

    void del(Type& item, int row, bool queued = false)
    {
        bridge::instance().increment_load();

        net_manager::instance().deleteToKey(key(item).c_str(),
            [this, &item, row, queued](const QJsonValue& rep)
            {
                using namespace crudpp;

                const auto id{t_trait<T>::pk_value(item.get_aggregate())};

                for (auto* p : property_holder<T>::instances)
                    if (t_trait<T>::pk_value(p->get_aggregate()) == id)
                        p->clear();

                for (auto* m : instances)
                {
                    for (int i{0}; i < m->size(); i++)
                    {
                        auto& other_item{m->item_at(i)};

                        if (t_trait<T>::pk_value(other_item.get_aggregate()) == id)
                        {
                            m->removeItem(i);
                            break;
                        }
                    }
                }

                this->removeItem(row);
                bridge::instance().decrement_load();
                if (queued) bridge::instance().dequeue();
            },
            "Remove Error");
    }

    QVector<Type> m_items{};
};

} //namespace qt

template <typename T>
std::vector<qt::list_model<T>*> qt::list_model<T>::instances{};

#include "wobjectimpl.h"
W_OBJECT_IMPL(qt::list_model<T>, template <typename T>)
