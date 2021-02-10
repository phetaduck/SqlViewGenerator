#pragma once

#include "styleditemdelegate.h"
#include "models/sqltablemodel.h"
#include <QSqlRelation>

class ComboBoxItemDelegate : public StyledItemDelegate
{
    Q_OBJECT
public:
    using StyledItemDelegate::StyledItemDelegate;

    QString getText(const QModelIndex& index) const override;

    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;


    void setRelation(QSqlRelation relation);

    void setDb(QSqlDatabase db);

    QSqlDatabase getDb() const;

    SqlTableModel* getSqlModel() const;
    void setSqlModel(SqlTableModel* sqlModel);

protected:

    std::function<void(QWidget* editor)> m_commitLambda;

    SqlTableModel* m_sqlModel = nullptr;
    QSqlDatabase m_db;
    QSqlRelation m_relation;
};
