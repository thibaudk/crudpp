#pragma once

#include <QSortFilterProxyModel>

namespace qt
{
struct sort_filter : public QSortFilterProxyModel
{
    explicit sort_filter(QObject* parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    bool filter_accepts_strings(const QModelIndex& index) const;
};
} // namespace qt
