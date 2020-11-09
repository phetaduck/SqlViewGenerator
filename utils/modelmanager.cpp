#include "modelmanager.h"

void ModelManager::ValueFilterModel::setFilterByValue(const QVariant& value)
{
    m_filterValue = value;
    invalidate();
}

bool ModelManager::ValueFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    auto sourceData = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent).data();
    if (m_filterValue.isNull() || sourceData == m_filterValue) {
        return true;
    }
    return false;
}
