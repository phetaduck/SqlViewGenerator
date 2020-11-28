#pragma once

#include <QSettings>
#include "sqlviewgenerator.h"

class Settings : public QSettings
{
    Q_OBJECT

public:

    Settings();
    void setLastLogin(const QString & value);
    auto lastLogin() const -> QString;
    void setDbHostName(const QString & value);
    auto dbHostName() const -> QString;
    void setDbName(const QString & value);
    auto dbName() const -> QString;
    void setDbPort(int port);
    int dbPort() const;
    void setDbPass(const QString & value);
    auto dbPass() const -> QString;
    void setLastCommands(const QString & value);
    auto lastCommands() const -> QString;
    void setDBType(const QString & value);
    auto dbType() const -> QString;
    void setLastSqlFile(const QString& value);
    auto lastSqlFile() const -> QString;
    void setTableSettings(const QString& key, const SqlSettings& value);
    auto tableSettings(const QString& key) const -> SqlSettings;
    void setDefaultTableSettings(const SqlSettings& value);
    auto defaultTableSettings() const -> SqlSettings;
//    auto lastDatabase() -> QString;
//    void setLastDatabase(const QString& value);
//    auto lastSchema() -> QString;
//    void setLastSchema(const QString& value);

    /**
     * @brief метод установки пути к классификаторам
     * @param путь к классификаторам
     */
    void setClassifiersPath(const QString value);

    /**
     * @brief метод получения пути к классификаторам
     * @return путь к классификаторам
     */
    QString getClassifiersPath() const;

private:
    static const QString LastLoginKey;
    static const QString DbHostNameKey;
    static const QString DbNameKey;
    static const QString DbPortKey;
    static const QString DbPassKey;
    static const QString LastCommandsKey;
    static const QString DbTypeKey;
    static const QString LastSqlFileKey;
    static const QString TableSettingsKey;
    static const QString DefaultTableSettingsKey;
    static const QString ClassifiersPathKey;
};
