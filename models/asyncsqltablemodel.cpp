#include "asyncsqltablemodel.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include "utils/sqlsharedutils.h"

AsyncSqlTableModel::~AsyncSqlTableModel()
{
    watcher.disconnect();
    if (future.isRunning()) {
        future.cancel();
        /// @note необходимо дождаться завершения получения данных
        future.waitForFinished();
    }
}

bool AsyncSqlTableModel::submitAll()
{
    if (!isDirty()) {
        emit submitAllStarted();
        emit submitAllFinished();
        return false;
    } else if (future.isRunning()) {
        return false;
    }

    beginResetModel();
    m_syncRowsCache = std::move(m_rows);
    m_rows = {};
    endResetModel();

    connect(&watcher, &QFutureWatcher<bool>::finished, [this]() {
        if (future.result())
            m_dirty = false;
        beginResetModel();
        m_rows = std::move(m_syncRowsCache);
        m_syncRowsCache = {};
        watcher.disconnect();
        endResetModel();
        emit this->submitAllFinished();
    });


    future = QtConcurrent::run([this]() {
        auto db = ThreadingCommon::DBConn::instance()->db();
        if (!db.isOpen())
        {
            return false;
        }
        bool updateResult = performUpdate();
        bool deleteResult = performDelete();
        bool insertResult = performInsert();
        return updateResult && deleteResult && insertResult;
    });

    watcher.setFuture(future);

    return true;
}

bool AsyncSqlTableModel::select()
{
    emit selectStarted();
    setSelectedAtLeastOnce(true);
    return _internalSelect([this](){
        emit this->selectFinished();
    });
}

void AsyncSqlTableModel::revert()
{
    emit revertStarted();
    _internalSelect([this](){
        emit this->revertFinished();
    });
}

void AsyncSqlTableModel::revertAll()
{
    revert();
}

void AsyncSqlTableModel::setFilter(const QString& filter)
{
    m_filter = filter;
}

bool AsyncSqlTableModel::isSelectedAtLeastOnce() const
{
    return SqlTableModel::isSelectedAtLeastOnce() || future.isRunning();
}

QString AsyncSqlTableModel::prepareUpdateStatement() const
{
    QString out = "UPDATE " + m_tableName + " SET  ";
    for (const auto& field : m_fieldNames) {
        if (field != m_primaryKeys)
            out += field + " = :" + field + ",";
    }
    out.remove(out.size() - 1, 1);
    out += " WHERE " + m_primaryKeys + " = :" + m_primaryKeys + " RETURNING *;";
    return out;
}

bool AsyncSqlTableModel::_internalSelect(std::function<void ()> callback)
{
    if (future.isRunning()) return false;
    connect(&watcher, &QFutureWatcher<bool>::finished, [this, callback]() {
        if (future.result())
            m_dirty = false;
        beginResetModel();
        m_rows = std::move(m_syncRowsCache);
        m_syncRowsCache = {};
        watcher.disconnect();
        endResetModel();
        m_selectedAtLeastOnce = true;
        callback();
    });
    future = QtConcurrent::run([this]() {
#ifndef QT_NO_DEBUG
        QTime dieTime= QTime::currentTime().addSecs(1);
        while (QTime::currentTime() < dieTime) {}
#endif
        QString request = "SELECT * FROM " + m_tableName;
        if (!m_filter.isEmpty()) {
            request += " WHERE " + m_filter;
        }
        if (!m_sort.isEmpty()) {
            request += " ORDER BY " + m_sort;
        }
        auto db = ThreadingCommon::DBConn::instance()->db();
        if (!db.isOpen()) {
            return false;
        }
        QSqlQuery query{db};
        if (query.exec(request)) {
            m_syncRowsCache.clear();
            if (m_lazyInit) {
                loadFieldNames();
            }
            if (query.size()) {
                m_syncRowsCache.reserve(query.size());
                while (query.next()) {
                    RowPtr row = std::make_shared<RowStruct>();
                    row->rowData = query.record();
                    m_syncRowsCache.push_back(row);
                }
            }
            m_dirty = false;
            m_selectedAtLeastOnce = true;
            return true;
        }
        showTextMessage(query.lastError().text(),
                        FailedToSelectMessage);
        return false;
    });

    watcher.setFuture(future);
    return true;
}

bool AsyncSqlTableModel::performUpdate()
{
    bool out = true;
    QString request;
    QHash<int, RowPtr> updatedRows;
    auto syncCacheSize = (int)m_syncRowsCache.size();
    updatedRows.reserve(syncCacheSize);
    for (int r = 0; r < syncCacheSize; r++) {
        auto& row = m_syncRowsCache[r];
        if (row->status == RowStatus::Updated) {
            updatedRows[r] = row;
        }
    }
    if (!updatedRows.isEmpty())
    {
        auto request = prepareUpdateStatement();

        auto db = ThreadingCommon::DBConn::instance()->db();
        QSqlQuery query{db};
        out = query.prepare(request);
        if (!out) {
            showTextMessage(query.lastError().text(),
                            FailedToPrepareUpdateMessage);
            return false;
        }
        bool execResult = true;
        QStringList errors;
        for (auto it = updatedRows.begin(); it != updatedRows.end(); it++) {
            auto& uRow = it.value();
            for (const auto& field : m_fieldNames) {
                query.bindValue(":" + field, uRow->rowData.value(field));
            }
            execResult &= query.exec();
            if (execResult) {
                if (query.next()) {
                    uRow->rowData = query.record();
                    uRow->status = RowStatus::None;
                }
            } else {
                errors << query.lastError().text();
            }
        }
        if (!execResult) {
            QString errorsText = errors.join("\n");
            qDebug() << errorsText;
            showTextMessage(errorsText,
                            FailedToExecUpdateMessage);
        }
        out &= execResult;
    }

    return out;
}

bool AsyncSqlTableModel::performDelete()
{
    bool out = true;
    auto rowC = m_syncRowsCache.size();
    if (!rowC) return true;
    QString request;
    std::vector<decltype (rowC)> deletedRowNumbers;
    deletedRowNumbers.reserve(rowC--);
    for (auto row = m_syncRowsCache.rbegin(); row != m_syncRowsCache.rend(); row++, rowC--) {
        if (row->get()->status == RowStatus::Deleted) {
            deletedRowNumbers.push_back(rowC);
        }
    }
    if (deletedRowNumbers.size())
    {
        request = "DELETE FROM " + m_tableName + " WHERE " + m_primaryKeys + " = ?;";
        auto db = ThreadingCommon::DBConn::instance()->db();
        QSqlQuery query{db};
        out = query.prepare(request);
        if (!out) {
            showTextMessage(query.lastError().text(),
                            FailedToPrepareDeleteMessage);
        } else {
            bool execResult = true;
            QStringList errors;
            for (const auto& rowC : deletedRowNumbers) {
                auto id = m_syncRowsCache[rowC]->rowData.value(m_primaryKeys);
                query.bindValue(0, id);
                execResult = query.exec();
                if (!execResult) {
                    errors << query.lastError().text();
                } else {
                    m_syncRowsCache.erase(m_syncRowsCache.begin() + rowC);
                }
                out &= execResult;
            }
            if (!execResult) {
                QString errorsText = errors.join("\n");
                qDebug() << errorsText;
                showTextMessage(errorsText,
                                FailedToExecDeleteMessage);
            }
        }
    }
    return out;
}

bool AsyncSqlTableModel::performInsert()
{
    bool out = true;
    QString request;
    QHash<int, RowPtr> insertedRows;
    auto syncCacheSize = (int)m_syncRowsCache.size();
    for (int r = 0; r < syncCacheSize; r++) {
        auto& row = m_syncRowsCache[r];
        if (row->status == RowStatus::Inserted) {
            insertedRows[r] = row;
        }
    }
    if (!insertedRows.isEmpty())
    {
        request = "INSERT INTO " + m_tableName + " ( ";
        for (const auto& field : m_fieldNames) {
            if (field != m_primaryKeys)
                request += field + ",";
        }
        request.remove(request.size() - 1, 1);
        request += ") VALUES (";
        for (const auto& field : m_fieldNames) {
            if (field != m_primaryKeys)
                request += ":" + field + ",";
        }
        request.remove(request.size() - 1, 1);
        request += ") RETURNING *;";
        auto db = ThreadingCommon::DBConn::instance()->db();
        QSqlQuery query{db};
        out = query.prepare(request);
        if (!out) {
            showTextMessage(query.lastError().text(),
                            FailedToPrepareInsertMessage);
        } else {
            bool execResult = true;
            QStringList errors;
            for (auto it = insertedRows.begin(); it != insertedRows.end(); it++) {
                auto& iRow = it.value();
                for (const auto& field : m_fieldNames) {
                    if (field != m_primaryKeys)
                        query.bindValue(":" + field, iRow->rowData.value(field));
                }
                execResult &= query.exec();
                if (execResult) {
                    if (query.next()) {
                        iRow->rowData = query.record();
                        iRow->status = RowStatus::None;
                        m_lastInsertId = iRow->rowData.value(m_primaryKeys);
                    }
                } else {
                    errors << query.lastError().text();
                }
            }
            if (!execResult) {
                QString errorsText = errors.join("\n");
                qDebug() << errorsText;
                showTextMessage(errorsText,
                                FailedToExecInsertMessage);
            }
            out &= execResult;
        }
    }
    return out;
}

