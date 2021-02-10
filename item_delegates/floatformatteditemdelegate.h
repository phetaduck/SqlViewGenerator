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
    using StyledItemDelegate::StyledItemDelegate;

    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
};

