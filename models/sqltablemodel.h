#pragma once

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDebug>
#include <memory>
#include <functional>
#include <QHash>

using ParentModelClass = QAbstractTableModel;

class SqlTableModel : public ParentModelClass
{
    Q_OBJECT
    Q_PROPERTY(QString tableName READ tableName WRITE setTableName NOTIFY tableNameChanged)

public:

    using DefaultValueCallback = std::function<QVariant(int, QString)>;
    using DefaultValuesCollection = QHash<QString, DefaultValueCallback>;
    using FlagsCollection = QHash<int, bool>;
    using editable_column_list = QList<int>;

    enum class RowStatus
    {
        None,
        Inserted,
        Updated,
        Deleted,
    };

    struct RowStruct {
        RowStatus status = RowStatus::None;
        Qt::CheckState checkState = Qt::Unchecked;
        QSqlRecord rowData;
    };
    using RowPtr = std::shared_ptr<RowStruct>;
    using RowsCollection = std::vector<RowPtr>;

    explicit SqlTableModel(QObject *parent = nullptr,
                                     QSqlDatabase db = QSqlDatabase());
    virtual ~SqlTableModel();

    virtual QModelIndex findIndex(
            int column,
            const QVariant& value);
    virtual QModelIndex findIndex(
            const QString& fieldName,
            const QVariant& value);
    virtual QModelIndex findIndex(
            int resultColumn,
            int searchColumn,
            const QVariant& value);
    virtual QModelIndex findIndex(
            const QString& resultFieldName,
            const QString& searchFieldName,
            const QVariant& value);
    template<typename Condition>
    QModelIndexList find_if(
            int column,
            Condition&& condition,
            int role = Qt::DisplayRole) const
    {
        QModelIndexList out;
        for (int r = 0; r < rowCount(); r++) {
            auto currentIndex = index(r, column);
            auto data = currentIndex.data(role);
            if (std::forward<Condition>(condition)(data)) {
                out << currentIndex;
            }
        }
        return out;
    }
    template<typename Condition>
    QModelIndexList find_if(
            Condition&& condition) const
    {
        QModelIndexList out;
        for (int r = 0; r < rowCount(); r++) {
            auto currentIndex = index(r, 0);
            auto currentRecord = record(r);
            if (std::forward<Condition>(condition)(currentRecord)) {
                out << currentIndex;
            }
        }
        return out;
    }
    template<class T>
    RowPtr findRow(const QString& field, T value)
    {
        for (const auto& row : m_rows)
        {
            if (row->rowData.value(field).value<T>() == value)
                return row;
        }
        return nullptr;
    }
    template<class T = QVariant>
    RowPtr findRow(const QString& field, const QVariant& value)
    {
        for (const auto& row : m_rows)
        {
            QVariant cellValue = row->rowData.value(field);
            if (cellValue.canConvert(value.type())) {
                cellValue.convert(value.type());
                if (cellValue == value)
                    return row;
            }
        }
        return nullptr;
    }
    template<class InputIt1, class OutputIt>
    OutputIt findRows(QString field,
                      InputIt1 first1, InputIt1 last1,
                      OutputIt d_first)
    {
        while (first1 != last1) {
            auto row = findRow(field, *first1++);
            if (row) {
                *d_first++ = row;
            }
        }
        return d_first;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canEdit() const;
    void setCanEdit(bool canEdit);
    editable_column_list& alwaysEditableColumns();
    QString tableName() const;
    virtual void setTable(const QString &tableName);
    QVariant data(const QModelIndex& idx,
                  int role = Qt::DisplayRole) const override;
    virtual QVariant data(int row,
                          const QString& fieldName,
                          int role = Qt::DisplayRole);
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    virtual bool setData(int row,
                         const QString& fieldName,
                         const QVariant &value,
                         int role = Qt::EditRole);
    virtual bool setData(int row,
                         int column,
                         const QVariant &value,
                         int role = Qt::EditRole);
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    virtual QModelIndex index(int row, const QString& field, const QModelIndex &parent = {}) const;
    QModelIndex parent(const QModelIndex &index = {}) const override;
    bool isDirty() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    RowPtr rowAt(int row);
    int fieldIndex(const QString &fieldName) const;
    RowPtr prepareNewRow(int newRowIndex);
    bool insertRows(int row, int count,
            const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count,
            const QModelIndex &parent = QModelIndex()) override;
    QString filter() const;
    virtual void setFilter(const QString &filter);

    auto checkableColumns() const -> FlagsCollection;
    void setCheckableColumns(
            const FlagsCollection& checkableColumns);

    void setCheckableColumns(
            const QHash<QString, bool>& checkableColumns);

    auto checkStates() const -> QMap<QPersistentModelIndex, bool>;
    void setCheckStates(const QMap<QPersistentModelIndex, bool>& checkStates);
    QList<QSqlRecord> selectedRecords() const;
    bool isRowMarkedForDeletion(int row);

    virtual void setPrimaryKeys(const QStringList& fieldName);
    virtual auto getPrimaryKeys() -> const QStringList&;

    virtual auto getRegularKeys() -> const QStringList&;
    virtual void setRegularKeys(const QStringList& regularKeys);

    std::vector<QString> fieldsToShow() const;
    void setFieldsToShow(const std::vector<QString>& fieldsToShow);

    std::vector<QString> fieldsToHide() const;
    void setFieldsToHide(const std::vector<QString>& fieldsToHide);

    auto fieldNames() const -> const QStringList&;
    void setFieldNames(const QStringList& fieldNames);
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                               int role = Qt::EditRole) override;

    QSqlDatabase database() const;
    void setDatabase(QSqlDatabase db);

    QSqlTableModel::EditStrategy editStrategy() const;
    virtual void setEditStrategy(QSqlTableModel::EditStrategy strategy);
    bool insertRecord(int row, const QSqlRecord &record);
    bool isDirty(const QModelIndex &index) const;
    QSqlRecord	record() const;
    QSqlRecord	record(int row) const;
    virtual void revertRow(int row);
    bool setRecord(int row, const QSqlRecord &values);
    virtual void setSort(int column, Qt::SortOrder order);
    QVariant lastInsertId() const;
    void setFilterAndSelectIfNeeded(const QString& newFilter);

    auto getRows() const -> const RowsCollection&;
    void setRows(const RowsCollection& rowsCollection);

    void setAlwaysEditableColumns(const editable_column_list& alwaysEditableColumns);

    /** @brief returns true if select has already been called at least once */
    virtual bool isSelectedAtLeastOnce() const;
    void setSelectedAtLeastOnce(bool selectedAtLeastOnce);

public slots:
    void setTableName(const QString& tableName);
    virtual bool select();
    void revert() override;
    void revertAll();
    virtual bool submitAll();
    bool submit() override;

signals:
    void tableNameChanged(QString tableName);
    void selectFinished();
    void submitAllFinished();
    void revertFinished();
    void selectStarted();
    void submitAllStarted();
    void revertStarted();

protected:
    virtual bool _internalSelect(std::function<void()> callback);
    virtual auto performUpdate() -> bool;
    virtual auto performInsert() -> bool;
    virtual auto performDelete() -> bool;
    void loadFieldNames();
    auto getRowsWithStatus(RowStatus status) const -> RowsCollection;
    auto generateUpdateRequest(bool& ok) const -> QString;
    auto generateInsertRequest(bool& ok) const -> QString;
    virtual bool insertRows(int row, const RowsCollection& rows = {},
            const QModelIndex &parent = QModelIndex());
    virtual void showTextMessage(const QString& debugText,
                                 const QString& userText = {});
    virtual void loadPrimaryKeys();
    virtual auto makePrimaryKeysNamedFilter() -> QString;

    bool m_canEdit = false;
    FlagsCollection m_checkableColumns;
    QMap<QPersistentModelIndex, bool> m_checkStates;
    editable_column_list m_alwaysEditableColumns;
    QString m_tableName;
    bool m_lazyInit = false;
    QStringList m_fieldNames;
    RowsCollection m_rows;
    DefaultValuesCollection m_defaultValues;
    QSqlDatabase m_db;
    QString m_filter;
    QString m_sort;
    QStringList m_primaryKeys;
    QStringList m_regularKeys;
    QSqlTableModel::EditStrategy m_editStrategy = QSqlTableModel::OnManualSubmit;
    mutable QVariant m_lastInsertId;
    bool m_dirty = false;
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder;
    QMap<int, QVariant> m_headerData;
    QSqlRecord m_defaultRecord;
    bool m_selectedAtLeastOnce = false;
    QString m_bareTableName;
    QString m_schema;
};

