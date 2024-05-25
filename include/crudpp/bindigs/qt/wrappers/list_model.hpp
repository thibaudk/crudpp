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
            QString::fromStdString(table + "get error"),
            [this] () { setLoading(false); });
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
            QString::fromStdString(table + "search error"),
            [this] () { setLoading(false); },
            p.c_str());
    }
    W_INVOKABLE(search)

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

        setLoading(row, true);

        QJsonObject obj{};
        item.write(obj);

        // update if the item was already inserted
        if (item.inserted())
        {
            net_manager::instance().putToKey(key(item).c_str(),
                QJsonDocument{obj}.toJson(),
                [&item, row, this](const QJsonObject& rep)
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
                { setLoading(row, false);
                });
        }
        else // insert otherwise
        {
            net_manager::instance().postToKey(T::table(),
                QJsonDocument{obj}.toJson(),
                [&item, row, this](const QJsonObject& rep)
                {
                    item.read(rep);
                    setLoading(row, false);
                },
                "save error",
                [row, this]()
                { setLoading(row, false);
                });
        }
    }
    W_INVOKABLE(save)

    void remove(int row)
    {
        setLoading(row, true);

        auto& item{this->item_at(row)};

        // delete on the server if it exists
        if (item.inserted())
        {
            net_manager::instance().deleteToKey(key(item).c_str(),
                [this, &item, row](const QJsonValue& rep)
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
                    setLoading(row, false);
                },
                "Remove Error",
                [this, row] ()
                { setLoading(row, false);
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

    void loadingChanged() const
    W_SIGNAL(loadingChanged)

private:
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

    QVector<Type> m_items{};
};
} //namespace qt

template <typename T>
std::vector<qt::list_model<T>*> qt::list_model<T>::instances{};

#include "wobjectimpl.h"
W_OBJECT_IMPL(qt::list_model<T>, template <typename T>)
