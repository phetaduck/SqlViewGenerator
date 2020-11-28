#include "sqlfiltermodel.h"
#include "sqltablemodel.h"

void SqlFilterModel::setSourceModel(SqlTableModel* sourceModel)
{
    m_sourceModel = sourceModel;
    QSortFilterProxyModel::setSourceModel(m_sourceModel);
}

bool SqlFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    bool out = m_sourceModel != nullptr;
    if (out) {
        if (m_filter) {
            out = m_filter(m_sourceModel, sourceRow, sourceParent);
        } else {
            for (auto it = m_filterColumns.begin(); it != m_filterColumns.end(); it++) {
                auto sourceValue = m_sourceModel->data(sourceRow, it.key());
                if (sourceValue == it.value()) {
                    out &= true;
                }
            }
        }
    }
    return out;
}

auto SqlFilterModel::filterColumns() -> QHash<QString, QVariant>&
{
    return m_filterColumns;
}
