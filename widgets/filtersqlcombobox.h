#pragma once

#include "sqlcombobox.h"

/**
 * @class FilterSqlComboBox
 * комбо бокс для работы с произвольной таблицей базы данных
 * и возможностью фильтрации
 * работа настраивается через QSqlRelation
 * QSqlRelation::tableName() имя таблицы
 * QSqlRelation::indexColumn() столбец возвращаемого значения
 * QSqlRelation::displayColumn() стобец отображаемого значения
 */

class QSortFilterProxyModel;

class FilterSqlComboBox : public SqlComboBox
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор
     * @param parent указатель на родительский элемент
     */
    explicit FilterSqlComboBox(QWidget *parent = nullptr);

    /** @note Перегруженные метод базового класса */
    void setModelColumn(int visibleColumn);
    /** @note Перегруженные метод базового класса */
    void setSqlData(SearchableSqlTableModel* model,
                    const QSqlRelation& sqlRelation) override;
    /** @note Перегруженные метод базового класса */
    QModelIndex selectedIndex() const override;
    ///@brief Установка текста-заполнителя
    void setPlaceholderText(const QString &placeholderText);

private:
    QSortFilterProxyModel* m_proxyModel = nullptr; ///< Прокси модель
    QSortFilterProxyModel* m_proxyModel1 = nullptr; ///< Прокси модель, поняти не имею зачем она была Игорю Б., но времени рабираться нет
    QCompleter *m_completer = nullptr; ///< Автокомплитер
};

