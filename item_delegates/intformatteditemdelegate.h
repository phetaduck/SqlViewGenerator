#pragma once

#include "styleditemdelegate.h"

class IntFormattedItemDelegate : public StyledItemDelegate
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

