#pragma once

#include "styleditemdelegate.h"
#include "models/sqltablemodel.h"
#include <QSqlRelation>

/**
 * @class ComboBoxItemDelegate
 * делегат с комбобоксом
 */
class ComboBoxItemDelegate : public StyledItemDelegate
{
    Q_OBJECT
public:
    /**
     * @brief унаследованные конструкторы */
    using StyledItemDelegate::StyledItemDelegate;

    /** перегруженный метод базового класса */
    QString getText(const QModelIndex& index) const override;

    /** перегруженный метод базового класса */
    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    /** перегруженный метод базового класса */
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    /** перегруженный метод базового класса */
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    /** перегруженный метод базового класса */
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;


    /** перегруженный метод базового класса */
    void setRelation(QSqlRelation relation);

    /** перегруженный метод базового класса */
    void setDb(QSqlDatabase db);

    /** перегруженный метод базового класса */
    QSqlDatabase getDb() const;

    SqlTableModel* getSqlModel() const;
    void setSqlModel(SqlTableModel* sqlModel);

protected:

    std::function<void(QWidget* editor)> m_commitLambda;

    SqlTableModel* m_sqlModel = nullptr; ///< SQL модель для комбобокса
    QSqlDatabase m_db; ///< база данных
    QSqlRelation m_relation; ///< SQL отношение
};
