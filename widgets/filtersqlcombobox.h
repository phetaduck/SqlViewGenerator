#pragma once

#include "sqlcombobox.h"

class QSortFilterProxyModel;

class FilterSqlComboBox : public SqlComboBox
{
    Q_OBJECT
public:
    explicit FilterSqlComboBox(QWidget *parent = nullptr);
    void setModelColumn(int visibleColumn);
    void setSqlData(SqlTableModel* model,
                    const QSqlRelation& sqlRelation) override;
    QModelIndex selectedIndex() const override;
    void setPlaceholderText(const QString &placeholderText);

private:
    QSortFilterProxyModel* m_proxyModel = nullptr;
    QSortFilterProxyModel* m_proxyModel1 = nullptr;
    QCompleter *m_completer = nullptr;
};

