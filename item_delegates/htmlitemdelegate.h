#pragma once

#include "styleditemdelegate.h"

class HTMLItemDelegate : public StyledItemDelegate
{
protected:
    using StyledItemDelegate::StyledItemDelegate;

    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};
