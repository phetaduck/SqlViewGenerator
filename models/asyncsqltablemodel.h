#pragma once

#include "sqltablemodel.h"

#include <QFutureWatcher>
#include <QtConcurrent>

#include <functional>

class AsyncSqlTableModel : public SqlTableModel
{
    Q_OBJECT
public:
    using SqlTableModel::SqlTableModel;
    virtual ~AsyncSqlTableModel();
    bool submitAll() override;
    bool select() override;
    void revert() override;
    void revertAll();
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
    bool performUpdate() override;
    bool performDelete() override;
    bool performInsert() override;
    QString prepareUpdateStatement() const;

    SqlTableModel::RowsCollection m_syncRowsCache;

    QFutureWatcher<bool> watcher;
    QFuture<bool> future;
    bool _internalSelect(std::function<void()> callback) override;

};
