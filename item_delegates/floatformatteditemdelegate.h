#pragma once

#include "styleditemdelegate.h"

/**
 * @class FloatFormattedItemDelegate
 * Делегат с кастомным рисованием и валидацией
 */
class FloatFormattedItemDelegate : public StyledItemDelegate
{
    Q_OBJECT
public:
    /**
     * @brief унаследованные конструкторы */
    using StyledItemDelegate::StyledItemDelegate;

    /** перегруженный метод базового класса */
    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    /** перегруженный метод базового класса */
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
};

