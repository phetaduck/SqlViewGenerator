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

/**
 * @brief Пространство имен для инкапсуляции данных связанных с классификаторами
 */
namespace Classifiers {

/**
 * @struct Структура настроек таблицы классификатора
 */
struct ClassifierSettings
{
    QString TableName; ///< Имя таблицы
    std::vector<int> HiddenColumns = {}; ///< Колонки не показываемые пользователю
    QHash<int, QString> Headers = {}; ///< Заголовки колонок
    QHash<int, QSqlRelation> Relations = {}; ///< Словарь отношений в реляционных таблицах
    QSqlTableModel::EditStrategy EditStrategy =
            QSqlTableModel::OnManualSubmit; ///< Стратегия редактирования
    std::pair<int, Qt::SortOrder> Sorting =
    {0, Qt::SortOrder::AscendingOrder}; ///< Сортировка
};

struct TableViewSettings
{
    QString TableName; ///< Имя таблицы
    std::vector<QString> HiddenColumns = {}; ///< Колонки не показываемые пользователю
    QHash<QString, QString> Headers = {}; ///< Заголовки колонок
    QHash<QString, QSqlRelation> Relations = {}; ///< Словарь отношений в реляционных таблицах
    QSqlTableModel::EditStrategy EditStrategy =
            QSqlTableModel::OnManualSubmit; ///< Стратегия редактирования
    std::pair<QString, Qt::SortOrder> Sorting =
    {"id", Qt::SortOrder::AscendingOrder}; ///< Сортировка
};

using ClassifiersListType = std::vector<std::pair<QString, ClassifierSettings>>;

static std::vector<std::pair<QString, ClassifierSettings>> KnownClassifiers;

auto LoadClassifiers (const QString& filePath) -> decltype (KnownClassifiers)&;

void SaveClassifiers (const QString& filePath);

void SaveClassifiers(const ClassifiersListType& classifiers,
                     const QString& filePath);

auto GetClassifiers () -> decltype (KnownClassifiers)&;

}

template <> inline
void JSON::fromJsonValue(const QJsonValue& json, Classifiers::ClassifierSettings& out)
{
    qDebug() << json.toString();
    QJsonObject object = json.toObject();
    JSON::fromJsonValue(object.value("TableName"), out.TableName);
    JSON::fromJsonValue(object.value("Headers"), out.Headers);
    JSON::fromJsonValue(object.value("HiddenColumns"), out.HiddenColumns);
    JSON::fromJsonValue(object.value("Relations"), out.Relations);
    JSON::fromJsonValue(object.value("EditStrategy"), out.EditStrategy);
    JSON::fromJsonValue(object.value("Sorting"), out.Sorting);
}

template <> inline
QJsonValue JSON::toJsonValue(const Classifiers::ClassifierSettings& cSettings)
{
    QJsonObject out;
    out.insert("TableName", JSON::toJsonValue(cSettings.TableName));
    out.insert("HiddenColumns", JSON::toJsonValue(cSettings.HiddenColumns));
    out.insert("Headers", JSON::toJsonValue(cSettings.Headers));
    out.insert("Relations", JSON::toJsonValue(cSettings.Relations));
    out.insert("EditStrategy", JSON::toJsonValue(cSettings.EditStrategy));
    out.insert("Sorting", JSON::toJsonValue(cSettings.Sorting));
    return out;
}

Q_DECLARE_METATYPE(Classifiers::ClassifierSettings);
