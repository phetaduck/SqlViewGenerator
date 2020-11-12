#include "sqltablemodel.h"
#include <algorithm>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlIndex>
#include <QMessageBox>
#include <QApplication>

#include "utils/sqlsharedutils.h"

#include <QDebug>

SqlTableModel::SqlTableModel(QObject* parent,
                                                 QSqlDatabase db) :
    ParentModelClass(parent), m_db(db)
{}

SqlTableModel::~SqlTableModel() = default;

QModelIndex SqlTableModel::findIndex(
        int column,
        const QVariant& value)
{
    if (column >= 0 && column < m_fieldNames.size()) {
        return findIndex(m_fieldNames[column], value);
    }
    return {};
}

QModelIndex SqlTableModel::findIndex(
        const QString& fieldName,
        const QVariant& value)
{
    QModelIndex out;
    auto column = m_fieldNames.indexOf(fieldName);
    if (column != -1) {
        for (int r = 0; r < rowCount(); r++)
        {
            auto row = m_rows[r];
            if (row->rowData.value(fieldName) == value) {
                out = index(r, column);
                break;
            }
        }
    }
    return out;
}

QModelIndex SqlTableModel::findIndex(
        int resultColumn,
        int searchColumn,
        const QVariant& value)
{
    auto out = findIndex(searchColumn, value);
    if (out.isValid()) {
        out = index(out.row(), resultColumn);
    }
    return out;
}

QModelIndex SqlTableModel::findIndex(
        const QString& resultFieldName,
        const QString& searchFieldName,
        const QVariant& value)
{
    auto out = findIndex(searchFieldName, value);
    if (out.isValid()) {
        out = index(out.row(), resultFieldName);
    }
    return out;

}

Qt::ItemFlags SqlTableModel::flags(const QModelIndex& index) const
{
    auto out = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    auto column = index.column();

    auto indexOf = m_alwaysEditableColumns.indexOf(column);
    if (m_canEdit || indexOf != -1)
    {
        out |= Qt::ItemIsEditable;
    }

    if (index.row() > -1 && index.row() < rowCount() &&
            m_rows[index.row()]->status == RowStatus::Inserted)
    {
        out |= Qt::ItemIsEditable;
    }

    if (m_checkableColumns.count(column) &&
            m_checkableColumns[column])
    {
        out |= Qt::ItemIsUserCheckable;
    }

    return out;
}

bool SqlTableModel::canEdit() const
{
    return m_canEdit;
}

void SqlTableModel::setCanEdit(bool canEdit)
{
    m_canEdit = canEdit;
}


auto SqlTableModel::alwaysEditableColumns()
-> SqlTableModel::editable_column_list&
{
    return m_alwaysEditableColumns;
}

QString SqlTableModel::tableName() const
{
    return m_tableName;
}

void SqlTableModel::setTable(const QString& tableName) {
    setTableName(tableName);
}

QVariant SqlTableModel::data(const QModelIndex& idx, int role) const
{
    QVariant out;
    if (idx.isValid()) {
        auto column = idx.column();
        auto row = idx.row();
        if (role == Qt::CheckStateRole && m_checkableColumns.count(column)) {
            if (m_checkableColumns[column])
            {
                if (m_checkStates[QPersistentModelIndex{idx}]) {
                    out = Qt::Checked;
                } else {
                    out = Qt::Unchecked;
                }
            }
        } else if ((role == Qt::DisplayRole || role == Qt::EditRole) &&
                   idx.row() < rowCount() &&
                   column < columnCount())
        {
            out = m_rows[row]->rowData.value(m_fieldNames[column]);
        }
    }
    return out;
}

QVariant SqlTableModel::data(int row,
                                       const QString& fieldName,
                                       int role)
{
    auto modelIndex = index(row, fieldName);
    return data(modelIndex, role);
}

bool SqlTableModel::setData(const QModelIndex& index,
                                      const QVariant& value,
                                      int role)
{
    if (index.isValid())
    {
        return setData(index.row(), index.column(), value, role);
    }
    return false;
}

bool SqlTableModel::setData(int row,
                                      const QString& fieldName,
                                      const QVariant& value,
                                      int role)
{
    auto modelIndex = index(row, fieldName);
    return setData(modelIndex, value, role);
}

bool SqlTableModel::setData(
        int row, int column, const QVariant& value, int role)
{
    if (row >= 0 &&
            row < rowCount() &&
            column >= 0 &&
            column < columnCount())
    {
        if (role == Qt::CheckStateRole) {
            auto rowToUpdate = m_rows[row];
            auto persistentModelIndex = QPersistentModelIndex{index(row, column)};
            m_checkStates[persistentModelIndex] = Qt::Checked == value;
        } else if (role == Qt::EditRole ||
                   role == Qt::DisplayRole)
        {
            auto rowToUpdate = m_rows[row];
            auto fieldName = m_fieldNames[column];
            if (rowToUpdate->rowData.value(fieldName) != value)
            {
                if (value.isNull()) {
                    rowToUpdate->rowData.setNull(fieldName);
                } else {
                    rowToUpdate->rowData.setValue(fieldName, value);
                }
                if (rowToUpdate->status == RowStatus::None) {
                    rowToUpdate->status = RowStatus::Updated;
                }
                m_dirty = true;
                emit dataChanged(index(row, column),
                                 index(row, column), {role});
                if (editStrategy() == QSqlTableModel::OnFieldChange) {
                    submitAll();
                }
            }
        } else {
            qDebug() << row << column << value << role << "role is not edit";
        }
        return true;
    }
    return false;
}

QModelIndex SqlTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return ParentModelClass::index(row, column, parent);
}

QModelIndex SqlTableModel::index(int row, const QString& field, const QModelIndex& parent) const
{
    return index(row, fieldIndex(field), parent);
}

QModelIndex SqlTableModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return {};
}

bool SqlTableModel::isDirty() const {
    return m_dirty;
}

int SqlTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return (int)m_rows.size();
}

int SqlTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return (int)m_fieldNames.size();
}

SqlTableModel::RowPtr SqlTableModel::rowAt(int row) {
    RowPtr out = nullptr;
    if (row > -1 && row < rowCount()) {
        out = m_rows[row];
    }
    return out;
}

int SqlTableModel::fieldIndex(const QString& fieldName) const {
    return m_defaultRecord.indexOf(fieldName);
}

SqlTableModel::RowPtr SqlTableModel::prepareNewRow(int newRowIndex) {
    RowPtr out = std::make_shared<RowStruct>();
    out->rowData = m_defaultRecord;
    out->status = RowStatus::Inserted;
    for (const auto& fieldName : m_fieldNames) {
        if (m_defaultValues[fieldName]) {
            out->rowData.setValue(fieldName, m_defaultValues[fieldName](
                                  newRowIndex, fieldName));
        }
    }
    return out;
}

bool SqlTableModel::insertRows(int row, int count, const QModelIndex&)
{
    RowsCollection newRows;
    newRows.reserve(count);
    int actualRow = row > -1 ? row : 0;
    for (int r = actualRow; r < row + count; r++) {
        newRows.push_back(prepareNewRow(r));
    }
    return insertRows(actualRow, newRows);
}

bool SqlTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || row + count > rowCount(parent)) return false;
    for (int i = 0; i < count; i++) {
        if (m_rows[row + i]->status == RowStatus::Inserted) {
            beginRemoveRows(parent, row + i, row + i);
            m_rows.erase(m_rows.begin() + row + i);
            endRemoveRows();
        } else {
            m_rows[row + i]->status = RowStatus::Deleted;
            m_dirty = true;
        }
    }
    return true;
}

QString SqlTableModel::filter() const {
    return m_filter;
}

void SqlTableModel::setFilter(const QString& filter) {
    m_filter = filter;
    if (m_selectedAtLeastOnce)
        select();
}

void SqlTableModel::setTableName(const QString& tableName)
{
    if (tableName != m_tableName) {
        m_tableName = tableName;
        if (!m_lazyInit) {
            getFieldNames();
        }
    }
    emit tableNameChanged(tableName);
}

bool SqlTableModel::select() {
    emit selectStarted();
    return _internalSelect([this](){
        emit this->selectFinished();
    });
}

void SqlTableModel::revert() {
    emit revertStarted();
    _internalSelect([this](){
        emit this->revertFinished();
    });
}

void SqlTableModel::revertAll() {
    revert();
}

bool SqlTableModel::submit() {
    return ParentModelClass::submit();
}

bool SqlTableModel::_internalSelect(std::function<void ()> callback)
{
    QString request = "SELECT * FROM " + m_tableName;
    if (!m_filter.isEmpty()) {
        request += " WHERE " + m_filter;
    }
    if (!m_sort.isEmpty()) {
        request += " ORDER BY " + m_sort;
    }
    QSqlQuery query{m_db};
    if (query.exec(request)) {
        beginResetModel();
        m_rows.clear();
        if (m_lazyInit) {
            getFieldNames();
        }
        if (query.size()) {
            m_rows.reserve(query.size());
            while (query.next()) {
                RowPtr row = std::make_shared<RowStruct>();
                row->rowData = query.record();
                m_rows.push_back(row);
            }
        }

        m_dirty = false;

        endResetModel();
        m_selectedAtLeastOnce = true;
        callback();
        return true;
    }
    callback();
#ifdef QT_DEBUG
    QMessageBox msgBox;
    msgBox.setText(query.lastError().text());
    msgBox.exec();
#endif
    return false;
}

bool SqlTableModel::submitAll()
{
    if (!isDirty()) {
        emit submitAllStarted();
        emit submitAllFinished();
        return false;
    }

    bool updateResult, deleteResult, insertResult;

    updateResult = performUpdate();

    deleteResult = performDelete();

    insertResult = performInsert();

    _internalSelect([this](){
                        emit this->submitAllFinished();
                    });

    return updateResult && deleteResult && insertResult;
}

auto SqlTableModel::getRows() const
-> const RowsCollection &
{
    return m_rows;
}

void SqlTableModel::setRows(const SqlTableModel::RowsCollection& rowsCollection)
{
    beginResetModel();
    m_rows = rowsCollection;
    endResetModel();
}

void SqlTableModel::getFieldNames() {
    m_fieldNames.clear();
    m_defaultRecord = m_db.record(m_tableName);
    if (m_primaryKey.isEmpty()) {
        m_primaryKey = m_defaultRecord.fieldName(0);
        if (m_primaryKey.isEmpty()) m_primaryKey = "id";
    }
    for (int i = 0; i < m_defaultRecord.count(); i++) {
        m_fieldNames.push_back(m_defaultRecord.fieldName(i));
    }
    if (m_sortColumn >= 0 &&
            m_sortColumn < (int)m_fieldNames.size() &&
            !m_fieldNames[m_sortColumn].isEmpty())
    {
        m_sort = m_fieldNames[m_sortColumn] +
                 (m_sortOrder == Qt::AscendingOrder ? " ASC" : " DESC");
    }
}

auto SqlTableModel::getRowsWithStatus(RowStatus status) const
-> RowsCollection
{
    RowsCollection out;
    for (const auto& row : m_rows) {
        if (row->status == status) {
            out.push_back(row);
        }
    }
    return out;
}

bool SqlTableModel::performUpdate()
{
    bool out = true;

    QString request;

    QHash<QString, QVariantList> varLists;

    for (const auto& row : m_rows) {
        if (row->status == RowStatus::Updated) {
            for (const auto& field : m_fieldNames) {
                varLists[field] << row->rowData.value(field);
            }
        }
    }

    if (varLists.size())
    {
        request = "UPDATE " + m_tableName + " SET  ";
        for (const auto& field : m_fieldNames) {
            if (field != m_primaryKey)
                request += field + " = " + "?,";
        }

        request.remove(request.size() - 1, 1);
        request += " WHERE " + m_primaryKey + " = :" + m_primaryKey + ";";

        QSqlQuery query{m_db};

        out = query.prepare(request);

        if (!out) {
            QMessageBox msgBox;
            msgBox.setText(query.lastError().text());
            msgBox.exec();
        }

        for (const auto& field : m_fieldNames) {
            if (field != m_primaryKey)
                query.addBindValue(varLists[field]);
        }
        if (varLists.count(m_primaryKey)) {
            query.addBindValue(varLists[m_primaryKey]);

            auto execResult = query.execBatch();

            if (!execResult) {
                qDebug() << query.lastError().text();
                QMessageBox msgBox;
                msgBox.setText(query.lastError().text());
                msgBox.exec();
            }

            out &= execResult;
        } else {
            QMessageBox msgBox;
            msgBox.setText(m_primaryKey + " не является основным ключом");
            msgBox.exec();
        }
    }

    return out;
}

bool SqlTableModel::performDelete()
{
    bool out = true;

    QString request;

    QVariantList varList;

    for (const auto& row : m_rows) {
        if (row->status == RowStatus::Deleted) {
            varList << row->rowData.value(m_primaryKey);
        }
    }

    if (!varList.isEmpty())
    {
        request = "DELETE FROM " + m_tableName + " WHERE " + m_primaryKey + " IN (?);";

        QSqlQuery query{m_db};

        out = query.prepare(request);

        if (!out) {
            QMessageBox msgBox;
            msgBox.setText(query.lastError().text());
            msgBox.exec();
        }

        query.addBindValue(varList);

        auto execResult = query.execBatch();

        if (!execResult) {
            qDebug() << query.lastError().text();
            QMessageBox msgBox;
            msgBox.setText(query.lastError().text());
            msgBox.exec();
        }

        out &= execResult;
    }
    return out;
}

void SqlTableModel::setDatabase(QSqlDatabase db)
{
    m_db = db;
}

void SqlTableModel::setAlwaysEditableColumns(const editable_column_list& alwaysEditableColumns)
{
    m_alwaysEditableColumns = alwaysEditableColumns;
}

bool SqlTableModel::performInsert()
{
    bool out = true;

    QString request;

    bool ok = false;

    QHash<QString, QVariantList> varLists;

    for (const auto& row : m_rows) {
        if (row->status == RowStatus::Inserted) {
            ok = true;
            for (const auto& field : m_fieldNames) {
                if (field != m_primaryKey)
                    varLists[field] << row->rowData.value(field);
            }
        }
    }

    if (ok)
    {
        request = "INSERT INTO " + m_tableName + " ( ";
        for (const auto& field : m_fieldNames) {
            if (field != m_primaryKey)
                request += field + ",";
        }
        request.remove(request.size() - 1, 1);
        request += ") VALUES (";
        for (const auto& field : m_fieldNames) {
            if (field != m_primaryKey)
                request += ":" + field + ",";
        }
        request.remove(request.size() - 1, 1);
        request += ");";

        QSqlQuery query{m_db};

        out = query.prepare(request);
        if (!out) {
            QMessageBox msgBox;
            msgBox.setText(query.lastError().text());
            msgBox.exec();
        }
        for (const auto& field : m_fieldNames) {
            if (field != m_primaryKey)
                query.addBindValue(varLists[field]);
        }

        auto execResult = query.execBatch();

        if (!execResult) {
            QMessageBox msgBox;
            msgBox.setText(query.lastError().text());
            msgBox.exec();
        }
        while (query.next()) {
            m_lastInsertId = query.value(0);
            qDebug() << m_lastInsertId;
        }

        out &= execResult;
    }
    return out;
}

QString SqlTableModel::generateUpdateRequest(bool& ok) const
{
    QString out;
    out = "UPDATE " + m_tableName + " AS O SET ";
    for (const auto& field : m_fieldNames) {
        if (field != m_primaryKey)
            out += field + " = N." + field + ",";
    }
    out.remove(out.size() - 1, 1);
    out += " FROM ( VALUES ";

    for (const auto& row : m_rows) {
        if (row->status == RowStatus::Updated) {
            out += "(";
            ok = true;
            for (const auto& field : m_fieldNames) {
                auto cellValue = row->rowData.value(field);
                if (!cellValue.isNull() && cellValue.isValid()) {
                    auto type = cellValue.type();
                    if (type == QVariant::String ||
                            type == QVariant::Uuid ||
                            type ==QVariant::Date ||
                            type ==QVariant::Time ||
                            type ==QVariant::DateTime)
                    {
                        out += "'" + cellValue.toString() + "'";
                    } else {
                        out += cellValue.toString();
                    }
                } else {
                    out += "'null'";
                }
                out += ",";
            }
            out.remove(out.size() - 1, 1);
            out += "),";
        }
    }

    out.remove(out.size() - 1, 1);
    out += " ) AS N ( ";
    for (const auto& field : m_fieldNames) {
        out += field + ",";
    }
    out.remove(out.size() - 1, 1);
    out += " ) ";
    out += "WHERE N."+ m_primaryKey + " = O." + m_primaryKey + ";";
    return out;
}

QString SqlTableModel::generateInsertRequest(bool& ok) const {
    QString out;
    out = "INSERT INTO " + m_tableName + " VALUES ";
    for (const auto& row : m_rows) {
        out += "(";
        if (row->status == RowStatus::Inserted) {
            ok = true;
            for (const auto& field : m_fieldNames) {
                auto cellValue = row->rowData.value(field);
                auto type = cellValue.type();
                if (type == QVariant::String ||
                        type == QVariant::Uuid ||
                        type ==QVariant::Date ||
                        type ==QVariant::Time ||
                        type ==QVariant::DateTime)
                {
                    out += "'" + cellValue.toString() + "'";
                } else {
                    out += cellValue.toString();
                }
                out +=  ",";
            }
        }
        out.remove(out.size() - 1, 1);
        out += "),";
    }
    out.remove(out.size() - 1, 1);
    out += ";";
    return out;
}

bool SqlTableModel::insertRows(int row,
                                         const RowsCollection& rows,
                                         const QModelIndex& parent)
{
    if (rows.empty()) return false;
    int actualRow = row;
    if (actualRow < 0) actualRow = 0;
    beginInsertRows(parent, actualRow, actualRow + (int)rows.size() - 1);
    if (actualRow >= rowCount(parent)) {
        m_rows.insert(std::end(m_rows), std::begin(rows), std::end(rows));
    } else {
        m_rows.insert(m_rows.begin() + actualRow, rows.begin(), rows.end());
    }
    m_dirty = true;
    endInsertRows();
    if (editStrategy() == QSqlTableModel::OnFieldChange) {
        submitAll();
    }
    return true;
}

void SqlTableModel::showTextMessage(const QString& debugText, const QString& userText)
{
#ifdef QT_DEBUG
    Q_UNUSED(userText);
    ThreadingCommon::dispatchToMainThread([=]() {
        QMessageBox msgBox;
        msgBox.setText(debugText);
        msgBox.exec();
    });
#else
    Q_UNUSED(debugText);
    if (userText.isEmpty()) return;
    ThreadingCommon::dispatchToMainThread([=]() {
        QMessageBox msgBox;
        msgBox.setText(userText);
        msgBox.exec();
    });
#endif
}

bool SqlTableModel::isSelectedAtLeastOnce() const
{
    return m_selectedAtLeastOnce;
}

void SqlTableModel::setSelectedAtLeastOnce(bool selectedAtLeastOnce)
{
    m_selectedAtLeastOnce = selectedAtLeastOnce;
}

auto SqlTableModel::fieldNames() const -> const QStringList&
{
    return m_fieldNames;
}

void SqlTableModel::setFieldNames(const QStringList& fieldNames)
{
    m_fieldNames = fieldNames;
}

QVariant SqlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant out;
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        if (section < (int)m_rows.size()) {
            auto row = m_rows[section];
            switch (row->status) {
            case RowStatus::Deleted :
                out = "-";
                break;
            case RowStatus::Updated :
                out = "*";
                break;
            case RowStatus::Inserted :
                out = "+";
                break;
            case RowStatus::None :
                out = "";
                break;
            }
        } else {
            out = section;
        }
    } else if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (m_headerData.contains(section)) {
            out = m_headerData[section];
        } else if (section < m_fieldNames.size()) {
            out = m_fieldNames[section];
        }
    } else {
        return ParentModelClass::headerData(section, orientation, role);
    }
    return out;
}

bool SqlTableModel::setHeaderData(int section,
                                            Qt::Orientation orientation,
                                            const QVariant& value, int role)
{
    if (orientation == Qt::Horizontal) {
        m_headerData[section] = value;
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return ParentModelClass::setHeaderData(section, orientation, role);
}

QSqlDatabase SqlTableModel::database() const
{
    return m_db;
}

QSqlTableModel::EditStrategy SqlTableModel::editStrategy() const
{
    return m_editStrategy;
}

void SqlTableModel::setEditStrategy(QSqlTableModel::EditStrategy strategy)
{
    m_editStrategy = strategy;
}

bool SqlTableModel::insertRecord(int row, const QSqlRecord& record) {
    auto newRowPtr = prepareNewRow(row);
    newRowPtr->rowData = record;
    return insertRows(row, {newRowPtr});
}

bool SqlTableModel::isDirty(const QModelIndex& index) const
{
    return m_rows[index.row()]->status != RowStatus::None;
}

QSqlRecord SqlTableModel::record() const
{
    return m_defaultRecord;
}

QSqlRecord SqlTableModel::record(int row) const
{
    auto out = record();
    if (row >= 0 && row < rowCount()) {
        out = m_rows[row]->rowData;
    }
    return out;
}

void SqlTableModel::revertRow(int row)
{
    Q_UNUSED(row);
}

bool SqlTableModel::setRecord(int row, const QSqlRecord& values)
{
    bool out = true;
    if (row >= 0 && row < rowCount()) {
        for (const auto& field : fieldNames()) {
            out &= setData(row, field, values.value(field));
        }
    }
    return out;
}

void SqlTableModel::setSort(int column, Qt::SortOrder order)
{
    m_sortColumn = column;
    m_sortOrder = order;
    if (!m_lazyInit || !m_fieldNames.isEmpty()) {
        if (m_sortColumn >= 0 &&
                m_sortColumn < m_fieldNames.size())
        {
            m_sort = m_fieldNames[m_sortColumn] +
                     (m_sortOrder == Qt::AscendingOrder ? " ASC" : " DESC");
        }
    }
}

QVariant SqlTableModel::lastInsertId() const
{
    return m_lastInsertId;
}

void SqlTableModel::setFilterAndSelectIfNeeded(const QString& newFilter) {
    if (!isSelectedAtLeastOnce() || filter() != newFilter) {
        setFilter(newFilter);
        select();
    }
}

QMap<QPersistentModelIndex, bool> SqlTableModel::checkStates() const
{
    return m_checkStates;
}

void SqlTableModel::setCheckStates(
        const QMap<QPersistentModelIndex, bool>& checkStates)
{
    m_checkStates = checkStates;
    for (auto it = m_checkStates.begin(); it != m_checkStates.end(); it++) {
        if (it.key().isValid()) {
            setData(it.key(),
                    it.value() ? Qt::Checked : Qt::Unchecked,
                    Qt::CheckStateRole);
        }
    }
}

QList<QSqlRecord> SqlTableModel::selectedRecords() const
{
    QList<QSqlRecord> out;
    for (auto it = m_checkStates.begin(); it != m_checkStates.end(); it++) {
        if (it.key().isValid()) {
            out << record(it.key().row());
        }
    }
    return out;
}

bool SqlTableModel::isRowMarkedForDeletion(int row)
{
    if (row < 0 || row >= (int)m_rows.size()) return false;
    return m_rows[row]->status == RowStatus::Deleted;
}

void SqlTableModel::setPrimaryKey(const QString& fieldName)
{
    m_primaryKey = fieldName;
}

auto SqlTableModel::checkableColumns() const
-> SqlTableModel::FlagsCollection
{
    return m_checkableColumns;
}

void SqlTableModel::setCheckableColumns(
        const FlagsCollection& checkableColumns)
{
    m_checkableColumns = checkableColumns;
}

void SqlTableModel::setCheckableColumns(
        const QHash<QString, bool>& checkableColumns)
{
    m_checkableColumns.clear();
    FlagsCollection checkableMap;
    for (auto it = checkableColumns.begin(); it != checkableColumns.end(); it++) {
        const auto& key = it.key();
        const auto& value = it.value();
        checkableMap[fieldIndex(key)] = value;
    }
    setCheckableColumns(checkableMap);
}
