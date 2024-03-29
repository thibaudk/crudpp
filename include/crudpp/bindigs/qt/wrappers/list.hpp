#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <wobjectdefs.h>

#include "model.hpp"

namespace qt
{
template <typename T>
class list final : public QObject
{
    W_OBJECT(list)

public:
    list(QObject* parent = nullptr)
        : QObject{parent}
    {}

    using Type = model<T>;

    static const constexpr auto table() { return Type::table(); }

    int size() const { return m_items.size(); }

    Type& item_at(int index) { return m_items[index]; }

    QVector<Type> items() const { return m_items; }

    void get()
    W_SIGNAL(get)

    void save(int row)
    W_SIGNAL(save, row)

    void select(int row)
    W_SIGNAL(select, row)

    void select_by(const QByteArray& roleName, const QVariant& value)
    W_SIGNAL(select_by, roleName, value)

    void loaded(int row)
    W_SIGNAL(loaded, row)

    void preItemsAppended(int number = 1)
    W_SIGNAL(preItemsAppended, number)
    void postItemsAppended()
    W_SIGNAL(postItemsAppended)

    void appendItems(int number = 1)
    {
        emit preItemsAppended(number);

        for (int i = 0; i < number; i++)
            m_items.emplace_back(Type{});

        emit postItemsAppended();
    }
    W_SLOT(appendItems, (int))

    void appendItem()
    {
        emit preItemsAppended(1);
        m_items.emplace_back(Type{});
        emit postItemsAppended();
    }
    W_SLOT(appendItem)

    void append(Type&& item)
    {
        emit preItemsAppended(1);
        m_items.emplace_back(item);
        emit postItemsAppended();
    }

    void preItemsRemoved(int first, int last)
    W_SIGNAL(preItemsRemoved, first, last)
    void postItemsRemoved()
    W_SIGNAL(postItemsRemoved)

    void dataChangedAt(int index)
    W_SIGNAL(dataChangedAt, index)

    void removeItems(int first, int last)
    {
        emit preItemsRemoved(first, last);

        m_items.erase(m_items.begin() + first,
                      std::next(m_items.begin() + last));

        emit postItemsRemoved();
    }
    W_SLOT(removeItems, (int, int))

    void removeItems(int number = 1)
    {
        const auto count = size();
        removeItems(count - number, count - 1);
    }
    W_SLOT(removeItems, (int))

    void removeItem(int index)
    {
        removeItems(index, index);
    }
    W_SLOT(removeItem, (int))

    void set_list(const QVector<Type>& list) { m_items = list; }

    QVector<Type>& get_list() { return m_items; }

    void clear()
    {
        emit preItemsRemoved(0, size() - 1);
        m_items.clear();
        emit postItemsRemoved();
    }
    W_INVOKABLE(clear)

    void addWith(const QJsonObject& obj)
    W_SIGNAL(addWith, obj)

    void remove(int row)
    W_SIGNAL(remove, row)

    void read(const QJsonArray& array)
    {
        clear();
        emit preItemsAppended(array.size());

        if (!array.empty())
            for (const auto& json : array)
                m_items.emplace_back(Type{json.toObject()});

        emit postItemsAppended();
    }

    void read(const QJsonObject& obj)
    {
        clear();

        emit preItemsAppended(1);
        m_items.emplace_back(Type{obj});
        emit postItemsAppended();
    }

    void read(const QByteArray& bytes)
    {
        const auto doc{QJsonDocument::fromJson(bytes)};

        if (doc.isArray())
            read(doc.array());
        else
            read(doc.object());
    }

    void write(QJsonArray& arr)
    {
        for (const auto& item : m_items)
        {
            QJsonObject obj{};
            item.wite(obj);
            arr.append(obj);
        }
    }

private:
    void checkCompleted();

    QVector<Type> m_items{};
};

} // namespace qt

#include <wobjectimpl.h>
W_OBJECT_IMPL(qt::list<T>, template <typename T>)
