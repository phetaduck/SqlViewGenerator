#include "styleditemdelegate.h"

#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QApplication>

void StyledItemDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const
{
    auto style = QApplication::style();
    QString text;
    if (Format) {
        text = Format(index.data());
    } else {
        text = getText(index);
    }
    auto rect = option.rect;
    painter->save();
    auto pal = option.palette;
    auto backgroundColor = index.data(Qt::BackgroundRole);
    if (backgroundColor.isValid() && backgroundColor.canConvert<QColor>()) {
        painter->fillRect(rect, backgroundColor.value<QColor>());
    }

    QStyleOptionButton cbOpt;
    cbOpt.rect = option.rect;
    cbOpt.rect.setLeft(cbOpt.rect.left() + 5);

    if (index.flags() & Qt::ItemIsUserCheckable) {
        if (index.data(Qt::CheckStateRole) == Qt::Checked) {
            cbOpt.state |= QStyle::State_On | QStyle::State_Enabled;
            style->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
        } else {
            cbOpt.state |= QStyle::State_Off | QStyle::State_Enabled;
            style->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
        }
    } else {
        if (IsValid && !IsValid(index)) {
            if (option.state & QStyle::State_Selected) {
                pal.setColor(QPalette::Window, Qt::darkRed);
            } else {
                pal.setColor(QPalette::Window, Qt::red);
            }
            pal.setColor(QPalette::Text, Qt::white);
            painter->fillRect(option.rect, pal.window());
        } else if (option.state & QStyle::State_Selected) {
            pal.setColor(QPalette::Text, Qt::white);
            painter->fillRect(option.rect, QBrush("#a5b0c0"));
        }
    }

    rect.setLeft(rect.left() + 5);
    style->drawItemText(painter, rect,
                        textAlignmentFlags(),
                        pal, true,
                        text, QPalette::Text);

    painter->restore();
}

bool StyledItemDelegate::editorEvent(QEvent *event,
                                     QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index)
{
    if (index.flags() & Qt::ItemIsUserCheckable) {
        if (event->type() == QEvent::MouseButtonRelease)
        {
            if (index.data(Qt::CheckStateRole) == Qt::Checked) {
                model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
            } else {
                model->setData(index, Qt::Checked, Qt::CheckStateRole);
            }
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

int StyledItemDelegate::textAlignmentFlags() const
{
    return m_textAlignmentFlags;
}

void StyledItemDelegate::setTextAlignmentFlags(int textAlignmentFlags)
{
    m_textAlignmentFlags = textAlignmentFlags;
}
