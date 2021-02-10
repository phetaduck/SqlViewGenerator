#pragma once

#include <QStyledItemDelegate>

class SqlTableModel;

using RelationModel = SqlTableModel;

class RelationalItemDelegate : public QStyledItemDelegate
{
public:
    RelationalItemDelegate(QObject *parent = nullptr);

    QString displayText(const QVariant &value, const QLocale &locale) const override;

    void setIndexColumn(const QString &indexColumn);

    void setDisplayColumn(const QString &displayColumn);

    void setModel(RelationModel *model);

private:
    RelationModel *m_model;
    QString m_indexColumn;
    QString m_displayColumn;
};

