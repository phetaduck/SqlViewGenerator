#pragma once

#include "styleditemdelegate.h"

/**
 * @class HTMLItemDelegate класс делегата рисования HTML
 * в ячейке
 */
class HTMLItemDelegate : public StyledItemDelegate
{
protected:
    /**
     * @brief унаследованные конструкторы */
    using StyledItemDelegate::StyledItemDelegate;

    /** перегруженный метод базового класса */
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    /** перегруженный метод базового класса */
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};
