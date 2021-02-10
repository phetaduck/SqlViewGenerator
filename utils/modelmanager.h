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

namespace ModelManager
{
    class ValueFilterModel : public QSortFilterProxyModel
    {
    public:

        using QSortFilterProxyModel::QSortFilterProxyModel;
        void setFilterByValue(const QVariant& value);

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        QVariant m_filterValue;
    };

    template<typename T>
    struct StaticCache {
        QHash<QString, std::shared_ptr<T>> ModelsMap;
    };
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

