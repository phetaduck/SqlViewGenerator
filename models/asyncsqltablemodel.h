#pragma once

#include "sqltablemodel.h"

#include <QFutureWatcher>
#include <QtConcurrent>

#include <functional>

/** @class AsyncSqlTableModel - асинхронная модель данных
 * Синхронизация данных выполняется вы отдельном потоке,
 * все сообщения об обновлении выполняются в основном потоке
 * Внутреннея хранилище сохраняет порядок и сами элементы
 * после синхронизации.
 * На время синхронизации модификация данных и запуск другой синхронизации
 * этой же модели будет проигнорирован.
 * Использование любой стратегии коммита кромей ручной, наверное не лучшая идея.
*/
class AsyncSqlTableModel : public SqlTableModel
{
    Q_OBJECT
public:
    using SqlTableModel::SqlTableModel;
    virtual ~AsyncSqlTableModel();

    /** @brief Переопределенный метод базового класса*/
    bool submitAll() override;

    /** @brief Переопределенный метод базового класса*/
    bool select() override;

    /** @brief Переопределенный метод базового класса*/
    void revert() override;

    /** @brief Переопределенный метод базового класса*/
    void revertAll();

    /** @brief Переопределенный метод базового класса,
     * в отличие от стандартного поведения, не вызывает select */
    void setFilter(const QString &filter) override;

    bool isSelectedAtLeastOnce() const override;

    QString FailedToSelectMessage = "Не удалось получить данные из базы данных.";
    QString FailedToPrepareInsertMessage = "Не удалось сформировать запрос в базу данных.";
    QString FailedToExecInsertMessage = "Не удалось добавить все записи в базу данных.";
    QString FailedToPrepareUpdateMessage = FailedToPrepareInsertMessage;
    QString FailedToExecUpdateMessage = "Не удалось сохранить все изменённые данные в базу данных.";
    QString FailedToPrepareDeleteMessage = FailedToPrepareInsertMessage;
    QString FailedToExecDeleteMessage = "Не удалось удалить все выбранные записи из базы данных.";

protected:
    /** перегруженный метод базового класса*/
    bool performUpdate() override;

    /** перегруженный метод базового класса*/
    bool performDelete() override;

    /** перегруженный метод базового класса*/
    bool performInsert() override;

    /** Создает запрос на обновление */
    QString prepareUpdateStatement() const;

    SqlTableModel::RowsCollection m_syncRowsCache; ///< Кэш строк на время синхронизации

    QFutureWatcher<bool> watcher; ///< Наблюдатель, по завершении загрузки данных, обновляет интерфейс
    QFuture<bool> future; ///< Объект таски, выполняет работу в отдельном потоке

    /** @brief Внутренний метод получения данных Выполняет работу асинхронно. */
    bool _internalSelect(std::function<void()> callback) override;

};
