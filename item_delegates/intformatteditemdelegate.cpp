#include "intformatteditemdelegate.h"

#include <QDebug>
#include <QSpinBox>
#include <QPainter>
#include <QApplication>

QWidget* IntFormattedItemDelegate::createEditor(QWidget* parent,
                                            const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor = new QSpinBox(parent);
    editor->setMaximum(INT_MAX);
    editor->setMinimum(0.0);
    return editor;
}

void IntFormattedItemDelegate::setEditorData(QWidget* editor,
                                         const QModelIndex& index) const
{
    auto spinBox = qobject_cast<QSpinBox*>(editor);

    if (!spinBox) return;

    auto ss = index.data().toString();
    ss.remove('\t');
    ss.remove(' ');
    spinBox->setValue(ss.toInt());
}
