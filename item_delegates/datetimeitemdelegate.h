#pragma once

#include "styleditemdelegate.h"

class QDateTimeEdit;

/**
 * @class SqlTableModel
 * модель с возможностью поиска
 */
class DateTimeItemDelegate : public StyledItemDelegate
{
    Q_OBJECT
public:
    /**
     * @brief унаследованные конструкторы */
    using StyledItemDelegate::StyledItemDelegate;

    /** Перегруженный метод базового класса*/
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    /** Перегруженный метод базового класса*/
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    /** Перегруженный метод базового класса*/
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    /** Перегруженный метод базового класса*/
    void setDisplayFormat(const QString& displayFormat);

protected:
    /** Перегруженный метод базового класса*/
    QString getText(const QModelIndex& index) const override;

private:
    void applyFormat(QDateTimeEdit* dateEdit) const;
    void applyFormat(QDateTimeEdit* dateEdit);

    QString m_displayFormat = "dd.MM.yyyy hh:mm";
};

