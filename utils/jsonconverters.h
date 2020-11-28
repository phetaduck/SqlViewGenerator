#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QHash>

#include <QSqlTableModel>
#include <QSqlRelation>

#include <vector>

namespace JSON {

template <typename T>
void fromJsonValue(const QJsonValue& value, T& out)
{
    out = value.toVariant().value<T>();
}

template <>
void fromJsonValue(const QJsonValue& value, QSqlTableModel::EditStrategy& out);
template <>
void fromJsonValue(const QJsonValue& value, QString& out);
template <>
void fromJsonValue(const QJsonValue& value, QSqlRelation& out);
template <typename T>
void fromJsonValue(const QJsonValue& json, typename std::vector<T>& out);
template <typename K, typename T>
void fromJsonValue(const QJsonValue& json, QHash<K, T>& out);
template <typename K, typename T>
void fromJsonValue(const QJsonValue& json, typename std::pair<K, T>& out);

template <typename T>
QJsonValue toJsonValue(const T& value)
{
    return QJsonValue{value};
}

template <>
QJsonValue toJsonValue(const QSqlRelation& value);
template <typename T>
QJsonValue toJsonValue(const typename std::vector<T>& vector);
template <typename K, typename T>
QJsonValue toJsonValue(const QHash<K, T>& value);
template <typename K, typename T>
QJsonValue toJsonValue(const typename std::pair<K, T>& value);

}

template <>
inline void JSON::fromJsonValue(const QJsonValue& value, QSqlTableModel::EditStrategy& out)
{
    out = QSqlTableModel::EditStrategy (
                                       value.toVariant().value<int>());
}

template <>
inline void JSON::fromJsonValue(const QJsonValue& value, QString& out)
{
    out = value.toString();
}

template <>
inline void JSON::fromJsonValue(const QJsonValue& value, QSqlRelation& out)
{
    QJsonObject object = value.toObject();
    out = QSqlRelation {
        object.value("tableName").toString(),
                object.value("indexColumn").toString(),
                object.value("displayColumn").toString(),

    };
}

template <typename T>
inline void JSON::fromJsonValue(const QJsonValue& json, typename std::vector<T>& out)
{
    QJsonArray array = json.toArray();
    out.clear();
    int vectorSize = array.size();
    out.resize(vectorSize);
    for (int i = 0; i < vectorSize; i++) {
        JSON::fromJsonValue(array.at(i), out[i]);
    }
}

template <typename K, typename T>
inline void JSON::fromJsonValue(const QJsonValue& json, QHash<K, T>& out)
{
    QJsonObject object = json.toObject();
    out.clear();
    for (auto it = object.begin(); it != object.end(); it++) {
        K key;
        JSON::fromJsonValue(it.key(), key);
        JSON::fromJsonValue(it.value(), out[key]);
    }
}

template <typename K, typename T>
inline void JSON::fromJsonValue(const QJsonValue& json, typename std::pair<K, T>& out)
{
    QJsonObject object = json.toObject();
    JSON::fromJsonValue(object.value("first"), out.first);
    JSON::fromJsonValue(object.value("second"), out.second);
}

template <>
inline QJsonValue JSON::toJsonValue(const QSqlRelation& value)
{
    QJsonObject out;
    out.insert("tableName", value.tableName());
    out.insert("indexColumn", value.indexColumn());
    out.insert("displayColumn", value.displayColumn());
    return out;
}

template <typename T>
inline QJsonValue JSON::toJsonValue(const typename std::vector<T>& vector)
{
    QJsonArray out;
    int vectorSize = int(vector.size());
    for (int i = 0; i < vectorSize; i++) {
        out.insert(i, JSON::toJsonValue(vector[i]));
    }
    return out;
}

template <typename K, typename T>
inline QJsonValue JSON::toJsonValue(const QHash<K, T>& value)
{
    QJsonObject out;
    for (auto it = value.begin(); it != value.end(); it++) {
        QString key = QString("%1").arg(it.key());
        out.insert(key, JSON::toJsonValue(it.value()));
    }
    return out;
}

template <typename K, typename T>
inline QJsonValue JSON::toJsonValue(const typename std::pair<K, T>& value)
{
    QJsonObject out;
    out.insert("first", JSON::toJsonValue(value.first));
    out.insert("second", JSON::toJsonValue(value.second));
    return out;
}
