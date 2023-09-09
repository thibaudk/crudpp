#pragma once

#include <QAbstractListModel>
#include <wobjectdefs.h>

#include "list.hpp"

namespace crudpp
{
template <typename T>
class list_model : public QAbstractListModel
{
    W_OBJECT(list_model)

public:
    explicit list_model(QObject* parent = nullptr)
        : QAbstractListModel{parent}
    {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid() || !m_list)
            return 0;

        return m_list->size();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || !m_list)
            return QVariant{};

        return m_list->items().at(index.row()).data(role);
    }

    bool setData(const QModelIndex& index,
                 const QVariant& value,
                 int role = Qt::EditRole) override
    {
        if (!m_list)
            return false;

        m_list->item_at(index.row()).setData(value, role);
        // emit dataChanged for both the curent role and the "flagged_for_update role"
        emit dataChanged(index, index,
                         QVector<int>() << role
                                        << model<T>::flagged_for_update_role());
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

    void setList(list<T>* newList)
    {
        beginResetModel();

        if (m_list)
            m_list->disconnect(this);

        m_list = newList;

        if (m_list)
        {
            connect(m_list, &list<T>::preItemsAppended,
                    this, [=](int number)
                    {
                        const int index = m_list->size();
                        beginInsertRows(QModelIndex(), index, index + number - 1);
                    });

            connect(m_list, &list<T>::postItemsAppended,
                    this, [=]()
                    { endInsertRows(); });

            connect(m_list, &list<T>::preItemsRemoved,
                    this, [=](int first, int last)
                    { beginRemoveRows(QModelIndex(), first, last); });

            connect(m_list, &list<T>::preItemsRemoved,
                    this, [=](int first, int last)
                    { beginRemoveRows(QModelIndex(), first, last); });

            connect(m_list, &list<T>::postItemsRemoved,
                    this, [=]()
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
        }

        endResetModel();
    }

    W_PROPERTY(list<T>*, list READ getList WRITE setList)

protected:
    list<T>* m_list{nullptr};

private:
    void setLoading(int row, bool value)
    {
        int role{model<T>::now_loading_role()};
        m_list->item_at(row).setData(value, role);
        emit dataChanged(index(row),
                         index(row),
                         QVector<int>() << role << model<T>::flagged_for_update_role());
    }
};

} //namespace crudpp

#include "wobjectimpl.h"
W_OBJECT_IMPL(crudpp::list_model<T>, template <typename T>)
