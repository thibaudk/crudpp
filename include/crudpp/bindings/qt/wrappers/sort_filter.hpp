#include <QSortFilterProxyModel>

#include <wobjectdefs.h>

namespace qt
{
class sort_filter : public QSortFilterProxyModel
{
    W_OBJECT(sort_filter)

public:
    explicit sort_filter(QObject* parent = nullptr);
    void filter_by_string(const QString& str);
    W_INVOKABLE(filter_by_string, (const QString&))
    void filter_by_variants(const QVariantList& vars);
    W_INVOKABLE(filter_by_variants, (const QVariantList&))

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    bool filter_accepts_strings(const QModelIndex& index) const;
    QList<QVariant> role_vals;
};
} // namespace qt
