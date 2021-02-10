#pragma once

#include "styleditemdelegate.h"

class QDateTimeEdit;

class DateTimeItemDelegate : public StyledItemDelegate
{
    Q_OBJECT
public:
    using StyledItemDelegate::StyledItemDelegate;

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void setDisplayFormat(const QString& displayFormat);

protected:
    QString getText(const QModelIndex& index) const override;

private:
    void applyFormat(QDateTimeEdit* dateEdit) const;
    void applyFormat(QDateTimeEdit* dateEdit);

    QString m_displayFormat = "dd.MM.yyyy hh:mm";
};

