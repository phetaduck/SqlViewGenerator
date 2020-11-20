#include "settings.h"

#include <QVariant>
#include <QDir>
#include "application.h"

const QString Settings::LastLoginKey = "auth/lastLogin";
const QString Settings::DbHostNameKey = "db/hostName";
const QString Settings::DbNameKey = "db/name";
const QString Settings::DbPortKey = "db/port";
const QString Settings::DbPassKey = "db/pass";
const QString Settings::LastCommandsKey = "db/last_commands";
const QString Settings::DbTypeKey = "db/type";
const QString Settings::LastSqlFileKey = "sql/type";


Settings::Settings()
    : QSettings(
          QSettings::IniFormat,
          QSettings::UserScope,
          Application::organizationName(),
          Application::applicationName())
{
}

void Settings::setLastLogin(const QString &lastLogin)
{
    setValue(LastLoginKey, lastLogin);
}

QString Settings::lastLogin() const
{
    return value(LastLoginKey, QStringLiteral("postgres")).toString();
}

void Settings::setDbHostName(const QString &dbHostName)
{
    setValue(DbHostNameKey, dbHostName);
}

QString Settings::dbHostName() const
{
    return value(DbHostNameKey, QStringLiteral("localhost")).toString();
}

void Settings::setDbName(const QString &dbName)
{
    setValue(DbNameKey, dbName);
}

QString Settings::dbName() const
{
    return value(DbNameKey, QStringLiteral("postgres")).toString();
}

void Settings::setDbPort(int port)
{
    setValue(DbPortKey, port);
}

int Settings::dbPort() const
{
    return value(DbPortKey, 5432).toInt();
}

void Settings::setDbPass(const QString& pass)
{
    setValue(DbPassKey, pass);
}

QString Settings::dbPass() const
{
    return value(DbPassKey, "postgres").toString();
}

void Settings::setLastCommands(const QString& lastCommands)
{
    setValue(LastCommandsKey, lastCommands);
}

QString Settings::lastCommands() const
{
    return value(LastCommandsKey, "").toString();
}

void Settings::setDBType(const QString &value)
{
    setValue(DbTypeKey, value);
}

auto Settings::dbType() const -> QString
{
    return value(DbTypeKey, "QPSQL").toString();
}

void Settings::setLastSqlFile(const QString &value)
{
    setValue(LastSqlFileKey, value);
}

auto Settings::lastSqlFile() const -> QString
{
    return value(LastSqlFileKey, QDir::currentPath()).toString();
}
