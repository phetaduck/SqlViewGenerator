#pragma once

#include <QStyledItemDelegate>

class QTableView;

class BaseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    QSize sizeHint(
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

protected:
    virtual QString getText(const QModelIndex& index) const;
};
