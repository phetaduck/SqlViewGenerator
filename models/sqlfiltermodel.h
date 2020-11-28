#pragma once

#include <QSortFilterProxyModel>
#include <functional>

class SqlTableModel;

class SqlFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    using FilterType =
    std::function<bool(SqlTableModel*, int sourceRow, const QModelIndex &sourceParent)>;

    void setSourceModel(QAbstractItemModel *) override {}
    virtual void setSourceModel(SqlTableModel* sourceModel);

    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

     FilterType m_filter; ///< Filter function, takes precedence over filterColumns

     auto filterColumns() -> QHash<QString, QVariant>&;

private:
     SqlTableModel* m_sourceModel = nullptr;

     QHash<QString, QVariant> m_filterColumns;
};

