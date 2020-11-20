#include "boolitemdelegate.h"

#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QApplication>
#include <QCheckBox>

void BoolItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto style = QApplication::style();

    painter->save();
    auto pal = option.palette;

    auto centeredOption = option;
    centeredOption.decorationAlignment = Qt::AlignCenter;

    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);
    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() +
                       option.rect.width()/2 - checkbox_rect.width()/2);

    if (index.data().toBool()) {
        checkboxstyle.state |= QStyle::State_On | QStyle::State_Enabled;
    } else {
        checkboxstyle.state |= QStyle::State_Off | QStyle::State_Enabled;
    }
    style->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);

    painter->restore();
}

QWidget* BoolItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto editor = new QCheckBox(parent);
    editor->setText("");
    return editor;
}

void BoolItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto checkBox = qobject_cast<QCheckBox*>(editor);
    if ( checkBox) {
        checkBox->setText("");
        checkBox->setChecked(index.data().toBool());
    }
}

void BoolItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto checkBox = qobject_cast<QCheckBox*>(editor);
    if (checkBox)
    {
        model->setData(index, checkBox->isChecked());
    }
}

void BoolItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);
    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

    //center
    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() +
                               option.rect.width()/2 - checkbox_rect.width()/2);

    editor->setGeometry(checkboxstyle.rect);
}
