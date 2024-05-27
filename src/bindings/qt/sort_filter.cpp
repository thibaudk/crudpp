#include <crudpp/bindigs/qt/wrappers/sort_filter.hpp>

namespace qt
{
sort_filter::sort_filter(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortRole(Qt::UserRole);
    sort(0);
}

bool sort_filter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
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
                for (const auto& [key, val] : roleNames().asKeyValueRange())
                {
                    auto d = sourceModel()->data(source_index, key);
                    if (d.typeId() == QMetaType::QString)
                    {
                        if (d.toString().contains(filterRegularExpression()))
                            return true;
                    }
                }
                return false;
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
        for (const auto& [key, val] : roleNames().asKeyValueRange())
        {
            auto d = sourceModel()->data(source_index, key);
            if (d.typeId() == QMetaType::QString)
            {
                if (d.toString().contains(filterRegularExpression()))
                    return true;
            }
        }
        return false;
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
} // namespace qt
