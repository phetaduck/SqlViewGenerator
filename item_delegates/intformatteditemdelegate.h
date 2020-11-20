#pragma once

#include "styleditemdelegate.h"

/**
 * @class IntFormattedItemDelegate
 * Делегат с кастомным рисованием и валидацией
 */
class IntFormattedItemDelegate : public StyledItemDelegate
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

