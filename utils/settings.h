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

private:
    static const QString LastLoginKey;
    static const QString DbHostNameKey;
    static const QString DbNameKey;
    static const QString DbPortKey;
    static const QString DbPassKey;
    static const QString LastCommandsKey;
    static const QString DbTypeKey;
    static const QString PipeNameKey;
};
