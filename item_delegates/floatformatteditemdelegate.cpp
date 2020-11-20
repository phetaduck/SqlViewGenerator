#include "floatformatteditemdelegate.h"

#include <QDebug>
#include <QDoubleSpinBox>
#include <QPainter>
#include <QApplication>
#include <cfloat>

QWidget* FloatFormattedItemDelegate::createEditor(QWidget* parent,
                                            const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor = new QDoubleSpinBox(parent);
    editor->setMaximum(DBL_MAX);
    editor->setMinimum(0.0);
    return editor;
}

void FloatFormattedItemDelegate::setEditorData(QWidget* editor,
                                         const QModelIndex& index) const
{
    auto spinBox = qobject_cast<QDoubleSpinBox*>(editor);

    if (!spinBox) return;

    auto ss = index.data().toString();
    ss.remove('\t');
    ss.remove(' ');
    spinBox->setValue(ss.toDouble());
}
