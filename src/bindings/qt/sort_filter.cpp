#include <wobjectimpl.h>

#include <crudpp/bindings/qt/wrappers/sort_filter.hpp>

namespace qt
{
sort_filter::sort_filter(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortRole(Qt::UserRole);
    sort(0);
}

void sort_filter::filter_by_string(const QString& str)
{
    role_vals.clear();
    setFilterFixedString(str);
}

void sort_filter::filter_by_variants(const QVariantList& vars)
{
    // the list has to be even
    // grouping pairs of roles and values
    if (vars.count() % 2)
        return;

    role_vals = vars;
    invalidateFilter();
}

int sort_filter::parent_row(int filtered_row) const
{
    QModelIndex fi{index(filtered_row, 0)};
    return mapToSource(fi).row();
}

bool sort_filter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (!role_vals.empty())
    {
        auto it{role_vals.begin()};
        while (it != role_vals.end())
        {
            QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
            auto key{it->toByteArray()};
            auto d = sourceModel()->data(index, roleNames().key(key));
            it++;
            auto val{*it};
            if (d != val)
                return false;
            it++;
        }

        return true;
    }

    if (filterRegularExpression().pattern().isEmpty())
        return true;

    int column_count = columnCount(sourceParent);
    if (filterKeyColumn() == -1)
    {
        for (int column = 0; column < column_count; ++column)
        {
            QModelIndex source_index = sourceModel()->index(sourceRow, column, sourceParent);
            if (filterRole() <= Qt::DisplayRole)
            {
                return filter_accepts_strings(source_index);
            }
            else
            {
                QString key = data(source_index, filterRole()).toString();
                if (key.contains(filterRegularExpression()))
                    return true;
            }
        }
        return false;
    }

    if (filterKeyColumn() >= column_count) // the column may not exist
        return true;
    QModelIndex source_index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
    if (filterRole() <= Qt::DisplayRole)
    {
        return filter_accepts_strings(source_index);
    }
    else
    {
        QString key = sourceModel()->data(source_index, filterRole()).toString();
        return key.contains(filterRegularExpression());
    }
}

bool sort_filter::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    QVariant l = source_left.model()->data(source_left, sortRole());
    QVariant r = source_right.model()->data(source_right, sortRole());
    return QVariant::compare(l, r) == QPartialOrdering::Less;
}

bool sort_filter::filter_accepts_strings(const QModelIndex& index) const
{
    for (const auto& [key, val] : roleNames().asKeyValueRange())
    {
        auto d = sourceModel()->data(index, key);
        if (d.typeId() == QMetaType::QString)
        {
            if (d.toString().contains(filterRegularExpression()))
                return true;
        }
    }
    return false;
}
} // namespace qt

W_OBJECT_IMPL(qt::sort_filter)
