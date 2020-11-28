#pragma once

#include <QString>
#include <QTextStream>
#include <QSqlRecord>
#include <QSqlField>

class SqlSettings {
public:
    QString table;
    QSqlRecord record;
    QString tableSchema;
    QString viewSchema;
    QString primaryKey;
    QString sortColumn;
    QString sortOrder;
    QString comment;
    QList<QVariant> fieldOrder;
    QString owner;
    bool dropView;
    bool dropFunctions;
    bool sortEnabled;

    inline
    QVariantMap toMap() const {
        return QVariantMap{
            {"table", table},
            {"tableSchema", tableSchema},
            {"primaryKey", primaryKey},
            {"sortColumn", sortColumn},
            {"viewSchema", viewSchema},
            {"sortOrder", sortOrder},
            {"comment", comment},
            {"fieldOrder", fieldOrder},
            {"owner", owner},
            {"dropView", dropView},
            {"dropFunctions", dropFunctions},
            {"sortEnabled", sortEnabled},
        };
    }

    static
    SqlSettings fromMap(const QVariantMap& map) {
        SqlSettings out;
        out.table = map["table"].toString();
        out.tableSchema = map["tableSchema"].toString();
        out.primaryKey = map["primaryKey"].toString();
        out.sortColumn = map["sortColumn"].toString();
        out.viewSchema = map["viewSchema"].toString();
        out.sortOrder = map["sortOrder"].toString();
        out.comment = map["comment"].toString();
        out.fieldOrder = map["fieldOrder"].toList();
        out.owner = map["owner"].toString();
        out.dropView = map["dropView"].toBool();
        out.dropFunctions = map["dropFunctions"].toBool();
        out.sortEnabled = map["sortEnabled"].toBool();
        return out;
    }
};

Q_DECLARE_METATYPE(SqlSettings);

class SqlViewGenerator
{
public:
    static QString viewSql(const SqlSettings& settings);

    static QString insetFunctionSql(const SqlSettings& settings);

    static QString insetTriggerSql(const SqlSettings& settings);

    static QString updateFunctionSql(const SqlSettings& settings);

    static QString updateTriggerSql(const SqlSettings& settings);

    static QString deleteFunctionSql(const SqlSettings& settings);

    static QString deleteTriggerSql(const SqlSettings& settings);

};

