#include "settings.h"

#include <QVariant>
#include "application.h"

const QString Settings::LastLoginKey = "auth/lastLogin";
const QString Settings::DbHostNameKey = "db/hostName";
const QString Settings::DbNameKey = "db/name";
const QString Settings::DbPortKey = "db/port";
const QString Settings::DbPassKey = "db/pass";
const QString Settings::LastCommandsKey = "db/last_commands";
const QString Settings::DbTypeKey = "db/type";
const QString Settings::PipeNameKey = "pipe/name";
const QString Settings::LocalServerNameKey = "pipe/createdName";

const QString Settings::LastRemoteProtocolNameKey = "remote/protocol";
const QString Settings::LastRemoteServerNameKey = "remote/server";
const QString Settings::LastRemoteAPINameKey = "remote/api";

const QString Settings::WatchFileNameKey = "skud/watchFile";


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

void Settings::setPipeName(const QString &value)
{
    setValue(PipeNameKey, value);
}

auto Settings::pipeName() const -> QString
{
    return value(PipeNameKey, "\\\\.\\pipe\\PipeName").toString();
}

void Settings::setLocalServer(const QString &value)
{
    setValue(LocalServerNameKey, value);
}

auto Settings::localServer() const -> QString
{
    return value(LocalServerNameKey, "\\\\.\\pipe\\LocalServer").toString();
}

void Settings::setLastRemoteProtocol(const QString& value)
{
    setValue(LastRemoteProtocolNameKey, value);
}

QString Settings::lastRemoteProtocol() const
{
    return value(LastRemoteProtocolNameKey, "http://").toString();
}

void Settings::setLastRemoteServer(const QString& value)
{
    setValue(LastRemoteServerNameKey, value);
}

QString Settings::lastRemoteServer() const
{
    return value(LastRemoteServerNameKey, "127.0.0.1").toString();

}

void Settings::setLastRemoteAPI(const QString& value)
{
    setValue(LastRemoteAPINameKey, value);
}

QString Settings::lastRemoteAPI() const
{
    return value(LastRemoteAPINameKey, "/api/external/incident").toString();

}

void Settings::setWatchFile(const QString& value)
{
    setValue(WatchFileNameKey, value);
}

auto Settings::watchFile() const -> QString
{
    return value(WatchFileNameKey,
                 "C://Users//Виктория//AppData//Roaming//IronLogic//GuardCmd").toString();
}


