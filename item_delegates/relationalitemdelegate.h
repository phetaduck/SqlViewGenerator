#ifndef RELATIONALITEMDELEGATE_H
#define RELATIONALITEMDELEGATE_H

#include <QStyledItemDelegate>

class SearchableSqlTableModel;

using RelationModel = SearchableSqlTableModel;

/**
 * @class RelationalItemDelegate
 * Класс делегата для отображения значений из связанных таблиц
 */
class RelationalItemDelegate : public QStyledItemDelegate
{
public:
    ///@brief Конструктор
    RelationalItemDelegate(QObject *parent = nullptr);

    ///@brief Переопределенный метод базового класса
    QString displayText(const QVariant &value, const QLocale &locale) const override;

    ///@brief Установка наименования колонки с идентификатором
    void setIndexColumn(const QString &indexColumn);

    ///@brief Установка наименования колонки с отображаемым значением
    void setDisplayColumn(const QString &displayColumn);

    ///@brief Установка модели с данными
    void setModel(RelationModel *model);

private:
    RelationModel *m_model;     ///< Модель с данными
    QString m_indexColumn;      ///< Наименование колонки с идентификатором
    QString m_displayColumn;    ///< Наименование колонки с отображаемым значением
};

#endif // RELATIONALITEMDELEGATE_H
