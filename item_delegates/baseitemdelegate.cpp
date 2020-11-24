#include "baseitemdelegate.h"

#include <QPainter>
#include <QDebug>

#include <cmath>

QString BaseItemDelegate::getText(const QModelIndex& index) const
{
    return index.data().toString();
}

QWidget* BaseItemDelegate::createEditor(
        QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const
{
    auto editor = QStyledItemDelegate::createEditor(parent, option, index);
    return editor;
}

QSize BaseItemDelegate::sizeHint(
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
