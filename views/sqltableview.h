#pragma once

#include <QTableView>

#include <QSqlRelation>

class SearchableSqlTableModel;

/**
 * @struct Структура настроек таблицы
 */

struct TableViewSettings
{
    QString TableName; ///< Имя таблицы
    std::vector<QString> HiddenColumns = {}; ///< Колонки не показываемые пользователю
    QHash<QString, QString> Headers = {}; ///< Заголовки колонок
    QHash<QString, QSqlRelation> Relations = {}; ///< Словарь отношений в реляционных таблицах
    QSqlTableModel::EditStrategy EditStrategy =
            QSqlTableModel::OnManualSubmit; ///< Стратегия редактирования
    std::pair<QString, Qt::SortOrder> Sorting; ///< Сортировка
};

/**
 * @class SqlTableView
 * Таблица специализированная для отображения и
 * редактирования таблицы базы данных
 */
class SqlTableView : public QTableView
{
    Q_OBJECT
public:
    /** @brief Наследования конструкторов базового класса*/
    using QTableView::QTableView;

    /** @brief Конструктор таблицы
     * @param settings настройки таблицы
     * @param база данных
     * @param родительский виджет
    */

    int selectedRow() const;

    /** @brief переопределенный метод базового класса
     * @param model модель данных
    */
    void setModel(QAbstractItemModel* model) override;

    /** @brief перегруженый метод базового класса
     * @param model модель данных
    */
    virtual void setModel(SearchableSqlTableModel* model);

    /** @brief вставить строку в таблицу
     * @param row номер строки
     * @param defaultValues - значения для столбцов по-умолчанию
    */
    void insertRow(int row,
                   const QHash<QString, QVariant>& defaultValues);

    /** @brief вставить строку в таблицу
     * @param row номер строки
     * @param defaultValues - значения для столбцов по-умолчанию
    */
    void insertRow(int row,
                   const QHash<int, QVariant>& defaultValues = {});

    /** @brief удалить строку из таблицы
     * @param row номер строки
    */
    void deleteRow(int row);

    /** @brief удалить выделенную строку из таблицы
    */
    void deleteSelectedRow();

    /** @brief SQL модель
     * @return текущую модель, может быть nullptr
    */
    SearchableSqlTableModel* sqlModel() const;

    /** @brief getter возможность удаления */
    bool canDelete() const;
    /** @brief setter возможность удаления */
    void setCanDelete(bool canDelete);

    /** @brief getter возможность вставки */
    bool canInsert() const;
    /** @brief setter возможность вставки */
    void setCanInsert(bool canInsert);

    /** @brief getter возможность восстановления */
    bool canRestore() const;
    /** @brief setter возможность восстановления */
    void setCanRestore(bool canRestore);

    /** @brief getter возможность редактирования */
    bool canEdit() const;
    /** @brief setter возможность редактирования */
    void setCanEdit(bool canEdit);

    /** @brief getter имени поля логического удаления */
    QString deletedFieldName() const;
    /** @brief getter имени поля логического удаления */
    void setDeletedFieldName(const QString& deletedFieldName);

#ifndef QT_NO_CONTEXTMENU
    /** @brief вызывается при запросе контекстного меню */
    void contextMenuEvent(QContextMenuEvent* event) override;
#endif // QT_NO_CONTEXTMENU

    /** @brief Инициализация таблицы
     * @param settings настройки таблицы
     * @param база данных
    */
    void init(
            const TableViewSettings& settings,
            QSqlDatabase db);

    /** @brief Снимает флаг удаления для выделенной строки */
    void restoreSelectedRow();

    bool showMenu() const;
    void setShowMenu(bool showMenu);

    /** @brief Снимает флаг удаления для строки
     * @param row - строка для восстановления
     */
    void restoreRow(int row);

    /** @brief Возвращает делегат для колонки, приведенный к заданному типу
     */
    template<typename DelegateType>
    DelegateType typedItemDelegateForColumn(int column) const {
        auto delegate = itemDelegateForColumn(column);
        return qobject_cast<DelegateType>(delegate);
    }

    /** @brief Сохраняет изменения в модели
     */
    void submit();

private:

    QAction* m_insertAboveAct = nullptr; ///< QAction вставки над текущей строкой
    QAction* m_insertBelowAct = nullptr; ///< QAction вставки под текущей строкой
    QAction* m_restoreAct = nullptr; ///< QAction восстановления логически удаленных элементов
    QAction* m_deleteAct = nullptr; ///< QAction удаления элементов
    QAction* m_revertAct = nullptr; ///< QAction отмены изменений
    QAction* m_commitAct = nullptr; ///< QAction сохранения изменений

    QString m_deletedFieldName = "deleted_at"; ///< Имя поля логического удаления
    QString m_deletedFacilityFieldName = "deleted"; ///< Имя поля логического удаления

    int m_lastSelectedRow = -1; ///< Последняя выделенная строка

    QMap<QString, SearchableSqlTableModel*> m_comboboxModels; ///< Кэш моделей делегатов

    SearchableSqlTableModel* m_sqlModel = nullptr; ///< SQL модель

    bool m_canDelete = true; ///< Флаг возможности удаления
    bool m_canInsert = true; ///< Флаг возможности вставки
    bool m_canRestore = false; ///< Флаг возможности восстановления
    bool m_canEdit = true; ///< Флаг возможности редактирования
    bool m_showMenu = true;

    /** @brief свзязь пунктов контекстного меню */
    void connectContextMenuActions();

    /** @brief удаление пунктов контекстного меню */
    void disconnectContextMenuActions();

    /** @brief создание пунктов контекстного меню */
    void createActions();
};
