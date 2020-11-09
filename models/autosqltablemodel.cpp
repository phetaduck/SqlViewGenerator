#include "autosqltablemodel.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include "utils/sqlsharedutils.h"

bool AutoSqlTableModel::_internalSelect(std::function<void ()> callback)
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
        if (callback)
            callback();
        emit newRecords(newRecordsList);
        if (m_rows.size()) {
            QTimer::singleShot(1000, this, [this]()
            {
                this->_internalSelect({});
            });
        }
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
                getFieldNames();
            }
            if (query.size()) {
                m_syncRowsCache.reserve(query.size());
                while (query.next()) {
                    RowPtr row = std::make_shared<RowStruct>();
                    row->rowData = query.record();
                    m_syncRowsCache.push_back(row);
                }
            }
            if (m_selectedAtLeastOnce) {
                newRecordsList.clear();
                for (const auto& row : m_syncRowsCache) {
                    auto id = row->rowData.value(m_primaryKey);
                    auto it = std::find_if(m_rows.begin(),
                                           m_rows.end(),
                                           [this, id](decltype (row) item)
                    {
                        return item->rowData.value(m_primaryKey) == id;
                    });
                    if (it == m_rows.end()) {
                        newRecordsList << row->rowData;
                    }
                }
            }
            return true;
        }
        showTextMessage(query.lastError().text(),
                        FailedToSelectMessage);
        return false;
    });

    watcher.setFuture(future);
    return true;
}
