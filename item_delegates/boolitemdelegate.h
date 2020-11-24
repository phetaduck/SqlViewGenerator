#pragma once

#include "styleditemdelegate.h"

/** @class BoolItemDelegate делегат для отображения чекбокса при работе с логическим типом данных.
 * Стандартный делегат отображает комбобокс.
*/
class BoolItemDelegate : public StyledItemDelegate
{
    Q_OBJECT
public:
    /** Наследуемые конструкторы */
    using StyledItemDelegate::StyledItemDelegate;

    /** Перегруженный метод */
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /** Перегруженный метод */
    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    /** Перегруженный метод */
    void setEditorData(QWidget* editor,
                       const QModelIndex& index) const override;

    /** Перегруженный метод */
    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    /** Перегруженный метод */
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

