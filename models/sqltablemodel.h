#pragma once

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDebug>
#include <memory>
#include <functional>
#include <QHash>

using ParentModelClass = QAbstractTableModel;

/**
 * @class SqlTableModel
 * модель Sql таблицы с возможностью поиска.
 * Можно использовать и без работы с БД.
 * Достаточно передать кусок другой модели и указать названия полей.
 */
class SqlTableModel : public ParentModelClass
{
    Q_OBJECT
    /// @brief свойство имени таблицы
    Q_PROPERTY(QString tableName READ tableName WRITE setTableName NOTIFY tableNameChanged)

public:

    using DefaultValueCallback = std::function<QVariant(int, QString)>;
    using DefaultValuesCollection = QHash<QString, DefaultValueCallback>;
    using FlagsCollection = QHash<int, bool>;
    using editable_column_list = QList<int>;

    /// перечисление статусов строк
    enum class RowStatus
    {
        None,
        Inserted,
        Updated,
        Deleted,
    };

    /// структура строки
    struct RowStruct {
        RowStatus status = RowStatus::None;
        Qt::CheckState checkState = Qt::Unchecked;
        QSqlRecord rowData;
    };
    using RowPtr = std::shared_ptr<RowStruct>;
    using RowsCollection = std::vector<RowPtr>;

    /// Конструктор
    explicit SqlTableModel(QObject *parent = nullptr,
                                     QSqlDatabase db = QSqlDatabase());
    /// Деструктор
    virtual ~SqlTableModel();

    /**
     * @brief найти индекс в указанной колонке для указанного значения
     * @param column колонка в которой будет осуществлен поиск
     * @param value искомое значение */
    virtual QModelIndex findIndex(
            int column,
            const QVariant& value);

    /**
     * @brief найти индекс в указанной колонке для указанного значения
     * @param fieldName имя колонки в которой будет осуществлен поиск
     * @param value искомое значение */
    virtual QModelIndex findIndex(
            const QString& fieldName,
            const QVariant& value);

    /**
     * @brief найти индекс в указанной колонке для указанного значения
     * @param resultColumn колонка к которой будет принадлежать указанный индекс
     * @param searchColumn колонка в которой будет осуществлен поиск
     * @param value искомое значение */
    virtual QModelIndex findIndex(
            int resultColumn,
            int searchColumn,
            const QVariant& value);

    /**
     * @brief найти индекс в указанной колонке для указанного значения
     * @param resultFieldName имя колонки к которой будет принадлежать указанный индекс
     * @param searchFieldName имя колонки в которой будет осуществлен поиск
     * @param value искомое значение */
    virtual QModelIndex findIndex(
            const QString& resultFieldName,
            const QString& searchFieldName,
            const QVariant& value);
    /**
     * @brief найти индексы в указанной колонке соответсвующие условию
     * @param column - колонка
     * @param condition - условие
     * @param value - значение
     */
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
    /**
     * @brief найти индексы соответсвующие условию
     * @param column - колонка
     * @param condition - условие
     * @param value - значение
     */
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

    /// поиск строки по значению в столбце
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

    /// специализация темплейта
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

    /// поиск строк в рейнже
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

    /** @brief Переопределенный метод базового класса*/
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** @brief getter доступности редактирования */
    bool canEdit() const;

    /** @brief setter доступности редактирования */
    void setCanEdit(bool canEdit);

    /** @brief колонки всегда доступные для редактирования
     * @return ссылка на вектор с колонками */
    editable_column_list& alwaysEditableColumns();

    QString tableName() const;

    virtual void setTable(const QString &tableName);

    /** @brief Переопределенный метод базового класса*/
    QVariant data(const QModelIndex& idx,
                  int role = Qt::DisplayRole) const override;

    /** @brief Перегруженный метод базового класса
     * @param row - номер строки
     * @param fieldName - имя поля
     * @param role - роль
     */
    virtual QVariant data(int row,
                          const QString& fieldName,
                          int role = Qt::DisplayRole);


    /** @brief Переопределенный метод базового класса*/
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;

    /** @brief Перегруженный метод базового класса
     * @param row - номер строки
     * @param fieldName - имя поля
     * @param value - значение
     * @param role - роль
     */
    virtual bool setData(int row,
                         const QString& fieldName,
                         const QVariant &value,
                         int role = Qt::EditRole);

    /** @brief Перегруженный метод базового класса
     * @param row - номер строки
     * @param column - номер столбца
     * @param value - значение
     * @param role - роль
     */
    virtual bool setData(int row,
                         int column,
                         const QVariant &value,
                         int role = Qt::EditRole);

    /** @brief Переопределенный метод базового класса*/
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;

    /** @brief Переопределенный метод базового класса*/
    virtual QModelIndex index(int row, const QString& field, const QModelIndex &parent = {}) const;

    /** @brief Переопределенный метод базового класса*/
    QModelIndex parent(const QModelIndex &index = {}) const override;

    /** @brief Переопределенный метод базового класса*/
    bool isDirty() const;

    /** @brief Переопределенный метод базового класса*/
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** @brief Переопределенный метод базового класса*/
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /** @brief строка */
    RowPtr rowAt(int row);

    /** @brief индекс поля */
    int fieldIndex(const QString &fieldName) const;

    /** @brief создать строку, не вставляет ее в таблицу */
    RowPtr prepareNewRow(int newRowIndex);

    /** @brief Переопределенный метод базового класса*/
    bool insertRows(int row, int count,
            const QModelIndex &parent = QModelIndex()) override;

    /** @brief Переопределенный метод базового класса*/
    bool removeRows(int row, int count,
            const QModelIndex &parent = QModelIndex()) override;

    /** @brief Переопределенный метод базового класса*/
    QString filter() const;

    /** @brief Переопределенный метод базового класса*/
    virtual void setFilter(const QString &filter);

    auto checkableColumns() const -> FlagsCollection;
    void setCheckableColumns(
            const FlagsCollection& checkableColumns);

    void setCheckableColumns(
            const QHash<QString, bool>& checkableColumns);

    auto checkStates() const -> QMap<QPersistentModelIndex, bool>;
    void setCheckStates(const QMap<QPersistentModelIndex, bool>& checkStates);

    /** @brief Метод возвращает выделенные записи в таблице
     */
    QList<QSqlRecord> selectedRecords() const;

    /** @brief Метод возвращает статус удаления строки
     * @param row - номер строки
     */
    bool isRowMarkedForDeletion(int row);

    void setPrimaryKey(const QString& fieldName);

    std::vector<QString> fieldsToShow() const;
    void setFieldsToShow(const std::vector<QString>& fieldsToShow);

    std::vector<QString> fieldsToHide() const;
    void setFieldsToHide(const std::vector<QString>& fieldsToHide);

    auto fieldNames() const -> const QStringList&;
    void setFieldNames(const QStringList& fieldNames);

    /** @brief Переопределенный метод базового класса*/
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /** @brief Переопределенный метод базового класса*/
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
    virtual void setSort(int column, Qt::SortOrder order);;

    /** @brief Последний вставленный айди, не работает с нашей БД*/
    QVariant lastInsertId() const;

    /** @brief Устанавливает фильтр и вызывает select,
     * если еще не иницализирована
     * или если новый фильтр отличается от старого
     * @param newFilter - новый фильтр
     */
    void setFilterAndSelectIfNeeded(const QString& newFilter);

    auto getRows() const -> const RowsCollection&;
    void setRows(const RowsCollection& rowsCollection);

    void setAlwaysEditableColumns(const editable_column_list& alwaysEditableColumns);

    bool isSelectedAtLeastOnce() const;
    void setSelectedAtLeastOnce(bool selectedAtLeastOnce);

public slots:
    /** @brief Переопределенный метод базового класса*/
    void setTableName(const QString& tableName);

    /** @brief Переопределенный метод базового класса*/
    virtual bool select();

    /** @brief Переопределенный метод базового класса*/
    void revert() override;

    /** @brief Переопределенный метод базового класса*/
    void revertAll();

    /** @brief Переопределенный метод базового класса*/
    virtual bool submitAll();

    /** @brief Переопределенный метод базового класса*/
    bool submit() override;

signals:
    /** @brief Сигнал изменения имени таблицы */
    void tableNameChanged(QString tableName);
    /** @brief Сигнал завершения получения данных. GUI thread. */
    void selectFinished();
    /** @brief Сигнал завершения отправки данных. GUI thread. */
    void submitAllFinished();
    /** @brief Сигнал завершения отмены изменений. GUI thread. */
    void revertFinished();
    /** @brief Сигнал начала получения данных. GUI thread. */
    void selectStarted();
    /** @brief Сигнал начала отправки данных. GUI thread. */
    void submitAllStarted();
    /** @brief Сигнал начала отмены изменений. GUI thread. */
    void revertStarted();

protected:
    /** @brief Внутренний метод получения данных Выполняет работу синхронно. */
    virtual bool _internalSelect(std::function<void()> callback);

    /** @brief Выполнить запрос на запись данных в БД */
    virtual auto performUpdate() -> bool;

    /** @brief Выполнить запрос на вставку новых записей */
    virtual auto performInsert() -> bool;

    /** @brief Выполнить запрос на удаление данных из БД */
    virtual auto performDelete() -> bool;

    /** @brief Имена полей */
    void getFieldNames();

    /** @brief Список указателей на строки с заданным статусом
     * @param status - статус*/
    auto getRowsWithStatus(RowStatus status) const -> RowsCollection; ///< Получить все поля по зананному статусу

    /** @brief Генерирует запрос на обновление записей */
    auto generateUpdateRequest(bool& ok) const -> QString;

    /** @brief Генерирует запрос на добавление записей */
    auto generateInsertRequest(bool& ok) const -> QString;

    /** @brief Вставляет набор строк с валидной структурой */
    virtual bool insertRows(int row, const RowsCollection& rows = {},
            const QModelIndex &parent = QModelIndex());

    /** @brief Выводит сообщение на экран.
     * @param debugText - Сообщение для отладочной сборки
     * @param userText - Сообщение для релизной сборки, пустая строка - не выводит ничего.
     * @note Всегда испольняется в GUI потоке.
     */
    virtual void showTextMessage(const QString& debugText,
                                 const QString& userText = {});

    bool m_canEdit = false; ///< флаг возможности редактирования

    FlagsCollection m_checkableColumns; ///< флаг возможности редактирования

    QMap<QPersistentModelIndex, bool> m_checkStates;

    editable_column_list m_alwaysEditableColumns; ///< список колонок всегда доступных для редактирования

    QString m_tableName; ///< Название таблицы

    bool m_lazyInit = false; ///< Флаг отложенной инициализации, позволяет максимально отсрочить коммуникацию с БД

    QStringList m_fieldNames; ///< имена полей, получаются из БД

    RowsCollection m_rows; ///< внутреннее хранилище строк

    DefaultValuesCollection m_defaultValues; ///< коллекция функций, позволяет инициализировать новые строки значениями по умолчанию

    QSqlDatabase m_db; ///< объект БД

    QString m_filter; ///< Фильтр, те же правила, что и у QSqlTableModel

    QString m_sort; ///< Сортировака, те же правила, что и у QSqlTableModel

    QString m_primaryKey; ///< Основной ключ, сначала попытается получить из БД, потом присвои значение по умолчанию - "id", можно задавать руками

    QSqlTableModel::EditStrategy m_editStrategy = QSqlTableModel::OnManualSubmit; ///< Стратегия редактирования, рализована не полностью

    mutable QVariant m_lastInsertId; ///< Последний вставленный айди

    bool m_dirty = false; ///< Флаг наличия изменений

    int m_sortColumn = -1; ///< Столбец сортировки

    Qt::SortOrder m_sortOrder; ///< Порядок сортировки

    QMap<int, QVariant> m_headerData; ///< Данные заголовков
    QSqlRecord m_defaultRecord; ///< Пустая запись по умолчанию, получается из БД первый раз, потом возвращает один и тот же объект

    bool m_selectedAtLeastOnce = false; ///< Был ли отправлен select хоть единожды

};

