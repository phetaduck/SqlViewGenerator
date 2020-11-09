#pragma once

#include <QComboBox>
#include <QSqlRelation>
#include "utils/modelmanager.h"

class SearchableSqlTableModel;

/**
 * @class SqlComboBox
 * комбо бокс для работы с произвольной таблицей базы данных
 * работа настраивается через QSqlRelation
 * QSqlRelation::tableName() имя таблицы
 * QSqlRelation::indexColumn() столбец возвращаемого значения
 * QSqlRelation::displayColumn() стобец отображаемого значения
 */
class SqlComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QVariant data READ data WRITE setData);
    Q_PROPERTY(QSqlRelation sqlRelation READ sqlRelation WRITE setSqlRelation NOTIFY sqlRelationChanged)
public:

    /** @brief Наследование конструкторов базового класса*/
    using QComboBox::QComboBox;

    /** @brief Перегруженный метод базового класса*/
    void setModel(QAbstractItemModel* model);

    /** @brief Выбранный элемент
     * @return индекс выбранного элемента
     */
    virtual QModelIndex selectedIndex() const;

    /** @brief Выбранный элемент
     * @param column столбец на который будет указывать индекс
     * @return индекс выбранного элемента
     */
    virtual QModelIndex selectedIndex(int column) const;

    /** @brief Выбранный элемент
     * @param fieldName название столбеца на который будет указывать индекс
     * @return индекс выбранного элемента
     */
    virtual QModelIndex selectedIndex(const QString& fieldName) const;

    /** @brief Выбрать элемент
     * @param data данные которые нужно найти в таблице
     */
    void setSelectedIndex(const QVariant& data);

    /** @brief SQL Модель данных
     * @return SQL модель в возможностью поиска, может быть nullptr
     */
    virtual SearchableSqlTableModel* sqlModel();

    /** @brief getter отношения */
    virtual auto sqlRelation() const -> const QSqlRelation&;

    /** @brief setter отношения и модели */
    virtual void setSqlData(SearchableSqlTableModel* sqlModel,
                    const QSqlRelation& sqlRelation);

    /** @brief getter Выбранный элемент */
    virtual QVariant data();
    /** @brief setter Выбранный элемент */
    virtual void setData(const QVariant& data);

public slots:
    void setSqlRelation(const QSqlRelation& sqlRelation);

signals:
    void sqlRelationChanged(QSqlRelation sqlRelation);

protected:

    QSqlRelation m_sqlRelation; ///< отношение в реляционной таблице
    SearchableSqlTableModel* m_sqlModel = nullptr; ///< SQL модель в возможностью поиска
};
