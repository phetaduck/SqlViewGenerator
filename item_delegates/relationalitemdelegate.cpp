#include "relationalitemdelegate.h"

#include "models/sqltablemodel.h"

RelationalItemDelegate::RelationalItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent),
    m_indexColumn("id"),
    m_displayColumn("name")
{
}

QString RelationalItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    Q_UNUSED(locale);
    QString displayText;
    if (m_model) {
        auto relationIndex = m_model->findIndex(m_displayColumn, m_indexColumn, value);
        displayText = relationIndex.data().toString();
    }
    return displayText;
}

void RelationalItemDelegate::setIndexColumn(const QString &indexColumn)
{
    m_indexColumn = indexColumn;
}

void RelationalItemDelegate::setDisplayColumn(const QString &displayColumn)
{
    m_displayColumn = displayColumn;
}

void RelationalItemDelegate::setModel(RelationModel *model)
{
    m_model = model;
}

