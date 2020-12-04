#pragma once

#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QFile>
#include <QTextCodec>
#include <unordered_map>
#include <vector>

#include "utils/application.h"
#include "jsonconverters.h"

namespace Classifiers {

struct SqlTableDescriptor
{
    QString TableName; ///< Имя таблицы
    std::vector<QString> ShownColumns = {}; ///< Колонки не показываемые пользователю
    QHash<QString, QString> Headers = {}; ///< Заголовки колонок
    QHash<QString, QSqlRelation> Relations = {}; ///< Словарь отношений в реляционных таблицах
    QSqlTableModel::EditStrategy EditStrategy =
            QSqlTableModel::OnManualSubmit; ///< Стратегия редактирования
    std::pair<QString, Qt::SortOrder> Sorting =
    {"order_index", Qt::SortOrder::AscendingOrder}; ///< Сортировка
};

template <typename T>
using vector_of_pairs = std::vector<std::pair<QString, T>>;

using TableDescriptorListType = vector_of_pairs<SqlTableDescriptor>;

static TableDescriptorListType KnownSqlTableDescriptors;

auto LoadSqlTableDescriptors (const QString& filePath) -> TableDescriptorListType&;

void SaveSqlTableDescriptors(const TableDescriptorListType& descriptors,
                     const QString& filePath);

void SaveSqlTableDescriptors (const QString& filePath);

auto GetSqlTableDescriptors () -> TableDescriptorListType&;

}

template <> inline
void JSON::fromJsonValue(const QJsonValue& json, Classifiers::SqlTableDescriptor& out)
{
    QJsonObject object = json.toObject();
    JSON::fromJsonValue(object.value("TableName"), out.TableName);
    JSON::fromJsonValue(object.value("Headers"), out.Headers);
    JSON::fromJsonValue(object.value("ShownColumns"), out.ShownColumns);
    JSON::fromJsonValue(object.value("Relations"), out.Relations);
    JSON::fromJsonValue(object.value("EditStrategy"), out.EditStrategy);
    JSON::fromJsonValue(object.value("Sorting"), out.Sorting);
}

template <> inline
QJsonValue JSON::toJsonValue(const Classifiers::SqlTableDescriptor& cSettings)
{
    QJsonObject out;
    out.insert("TableName", JSON::toJsonValue(cSettings.TableName));
    out.insert("ShownColumns", JSON::toJsonValue(cSettings.ShownColumns));
    out.insert("Headers", JSON::toJsonValue(cSettings.Headers));
    out.insert("Relations", JSON::toJsonValue(cSettings.Relations));
    out.insert("EditStrategy", JSON::toJsonValue(cSettings.EditStrategy));
    out.insert("Sorting", JSON::toJsonValue(cSettings.Sorting));
    return out;
}

Q_DECLARE_METATYPE(Classifiers::SqlTableDescriptor);
