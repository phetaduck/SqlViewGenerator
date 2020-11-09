#pragma once

#include "asyncsqltablemodel.h"

class AutoSqlTableModel : public AsyncSqlTableModel
{
    Q_OBJECT
public:
    using AsyncSqlTableModel::AsyncSqlTableModel;

signals:
    void newRecords(QList<QSqlRecord> newRecordsList);

protected:
    /** @brief Внутренний метод получения данных Выполняет работу асинхронно. */
    bool _internalSelect(std::function<void()> callback) override;
    QList<QSqlRecord> newRecordsList;
};

