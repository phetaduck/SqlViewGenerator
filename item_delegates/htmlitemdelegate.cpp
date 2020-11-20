#include "htmlitemdelegate.h"
#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QAbstractTextDocumentLayout>


void HTMLItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    auto text = getText(index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(text);

    auto style = QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &option, painter);
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &option);

    QAbstractTextDocumentLayout::PaintContext ctx;

    if (option.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, option.palette.color(QPalette::Active, QPalette::HighlightedText));
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize HTMLItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QTextDocument doc;
    doc.setHtml(getText(index));
    doc.setTextWidth(option.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}
