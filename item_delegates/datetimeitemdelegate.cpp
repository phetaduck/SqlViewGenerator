#include "datetimeitemdelegate.h"

#include <QDateTime>
#include <QDateTimeEdit>
#include <QCalendarWidget>

#include <QDebug>

QWidget* DateTimeItemDelegate::createEditor(QWidget* parent,
                                            const QStyleOptionViewItem& option,
                                            const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QDateTimeEdit* dateTimeEdit = new QDateTimeEdit(parent);
    QCalendarWidget* calendar = new QCalendarWidget;
    calendar->setLocale(QLocale::Russian);
    applyFormat(dateTimeEdit);
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setCalendarWidget(calendar);
    dateTimeEdit->setAlignment(Qt::AlignCenter);
    return dateTimeEdit;
}

void DateTimeItemDelegate::setEditorData(QWidget* editor,
                                         const QModelIndex& index) const
{
    auto dateTimeEdit = qobject_cast<QDateTimeEdit*>(editor);

    if (!dateTimeEdit) return;

    int row = index.row();

    if (row > -1) {
        auto ss = index.data().value<QDateTime>();
        dateTimeEdit->setDateTime(ss);
    }
}

void DateTimeItemDelegate::setModelData(QWidget* editor,
                                        QAbstractItemModel* model,
                                        const QModelIndex& index) const
{
    auto dateTimeEdit = qobject_cast<QDateTimeEdit*>(editor);
    if (dateTimeEdit) {
        QVariant newValue{dateTimeEdit->dateTime()};
        model->setData(index, newValue);
    } else {
#ifdef QT_DEBUG
        qDebug() << "editor is null";
#endif
    }
}

void DateTimeItemDelegate::setDisplayFormat(const QString& displayFormat) {
    m_displayFormat = displayFormat;
}

QString DateTimeItemDelegate::getText(const QModelIndex& index) const
{
    return index.data().toDateTime().toString(m_displayFormat);
}

void DateTimeItemDelegate::applyFormat(QDateTimeEdit* dateEdit) const
{
    dateEdit->setDisplayFormat(m_displayFormat);
}

void DateTimeItemDelegate::applyFormat(QDateTimeEdit* dateEdit)
{
    dateEdit->setDisplayFormat(m_displayFormat);
}
