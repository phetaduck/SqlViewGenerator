#pragma once

#include <QSettings>

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
    void setPipeName(const QString & value);
    auto pipeName() const -> QString;
    void setLocalServer(const QString & value);
    auto localServer() const -> QString;

    void setLastRemoteProtocol(const QString & value);
    auto lastRemoteProtocol() const -> QString;
    void setLastRemoteServer(const QString & value);
    auto lastRemoteServer() const -> QString;
    void setLastRemoteAPI(const QString & value);
    auto lastRemoteAPI() const -> QString;

    void setWatchFile(const QString& value);
    auto watchFile() const -> QString;

private:
    static const QString LastLoginKey;
    static const QString DbHostNameKey;
    static const QString DbNameKey;
    static const QString DbPortKey;
    static const QString DbPassKey;
    static const QString LastCommandsKey;
    static const QString DbTypeKey;
    static const QString PipeNameKey;
    static const QString LocalServerNameKey;
    static const QString LastRemoteProtocolNameKey;
    static const QString LastRemoteServerNameKey;
    static const QString LastRemoteAPINameKey;
    static const QString WatchFileNameKey;
};
