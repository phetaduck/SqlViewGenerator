#pragma once

#include "baseitemdelegate.h"

#include <functional>

class StyledItemDelegate : public BaseItemDelegate
{
    Q_OBJECT
public:
    using BaseItemDelegate::BaseItemDelegate;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

    std::function<bool(const QModelIndex&)> IsValid;
    std::function<QString(const QVariant& value)> Format;

    int textAlignmentFlags() const;
    void setTextAlignmentFlags(int textAlignmentFlags);

private:
    int m_textAlignmentFlags = Qt::AlignLeft |
                               Qt::AlignVCenter |
                               Qt::TextWordWrap;
};

