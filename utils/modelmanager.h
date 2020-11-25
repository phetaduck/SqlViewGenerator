#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include "models/asyncsqltablemodel.h"
#include "utils/sqlsharedutils.h"

#include <memory>
#include <type_traits>

class SqlTableModel;
class AsyncSqlTableModel;

/**
 * @class Менеджер шареных моделей.
 */
namespace ModelManager
{
    /**
     * Фильтрующая прокси модель
     */
    class ValueFilterModel : public QSortFilterProxyModel
    {
    public:

        using QSortFilterProxyModel::QSortFilterProxyModel;

        /**
         * @brief Отфильтровать пользователей по идентификатору значению
         */
        void setFilterByValue(const QVariant& value);

    protected:
        /** Переопределенный метод базового класса */
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        QVariant m_filterValue; ///< Значение для фильтрации
    };

    template<typename T>
    struct StaticCache {
        QHash<QString, std::shared_ptr<T>> ModelsMap;
    };
    /** Создает новую модель и добавляет ее в пул если еще не создана,
     * возвращает уже существующуюю в противном случае.
     * Если класс Т наследник SqlTableModel - все в порядке.
     * В противном случае возвращаемый тип не отпределен. */
    template<typename T>
    extern typename std::enable_if_t<std::is_base_of_v<SqlTableModel, T>, T*> /** */
    sharedSqlTableModel(const QString& tableName) {
        static StaticCache<T> cache = {};
        if (!cache.ModelsMap.contains(tableName)) {
            auto *conmgr = ThreadingCommon::DBConn::instance();
            auto db = conmgr->db();
            cache.ModelsMap[tableName] = std::make_shared<T>();
            cache.ModelsMap[tableName]->setDatabase(db);
            cache.ModelsMap[tableName]->setTableName(tableName);
        }
        return cache.ModelsMap[tableName].get();
    }

    /** @brief Функция для удобства, принимает ссылку на поле класса или переменную
     * и присваеивает ей указатель на вновь созданный элемент. */
    template<typename B>
    extern void sharedSqlTableModel(B& model, const QString& tableName) {
        auto cache = [](B& model, const QString& tableName) {
            return sharedSqlTableModel<std::remove_pointer_t<std::remove_reference_t<decltype (model)>>>(tableName);;
        };
        model = cache(model, tableName);
    }
    inline const QString EditableProjectsView = "public.editable_projects_view";
    inline const QString UsersView = "catalogs.users_view";
    inline const QString UserProjctsView = "public.user_projects_view";
};

