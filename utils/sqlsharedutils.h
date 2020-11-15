#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <QHash>
#include <QSqlDatabase>
#include <QIcon>
#include <QSqlRelation>
#include <unordered_map>
#include <QLocalSocket>

namespace NamedPipes {

inline auto socketErrors() -> std::unordered_map<int, QString> & {
    static std::unordered_map<int, QString> errors {
        { QLocalSocket::ConnectionRefusedError, "ConnectionRefusedError" },
        { QLocalSocket::PeerClosedError, "PeerClosedError" },
        { QLocalSocket::ServerNotFoundError, "ServerNotFoundError" },
        { QLocalSocket::SocketAccessError, "SocketAccessError" },
        { QLocalSocket::SocketResourceError, "SocketResourceError" },
        { QLocalSocket::SocketTimeoutError, "SocketTimeoutError" },
        { QLocalSocket::DatagramTooLargeError, "DatagramTooLargeError" },
        { QLocalSocket::ConnectionError, "ConnectionError" },
        { QLocalSocket::UnsupportedSocketOperationError, "UnsupportedSocketOperationError" },
        { QLocalSocket::UnknownSocketError, "UnknownSocketError" },
        { QLocalSocket::OperationError, "OperationError" },
    };
    return errors;
}

}

struct DBTypeDesc {
    QString DefaultDB;
    QSqlRelation DefaultRel;
};

inline QHash<QString, DBTypeDesc>& defaultDBs ()
{
    static QHash<QString, DBTypeDesc> out {
        {"QPSQL",
            {
                "postgres",
                {
                    "pg_database", "datname", "datname"
                }
            }
        },
        {"QMYSQL",
            {
                "information_schema",
                {
                    "schemata", "schema_name", "schema_name"
                }
            }
        }
    };

    return out;
}

inline QString defaultDB (const QString& db_type)
{
    return defaultDBs()[db_type].DefaultDB;
}

inline QSqlRelation defaultRelation (const QString& db_type)
{
    return defaultDBs()[db_type].DefaultRel;
}

namespace ThreadingCommon {

class DBConn {
public:
    static auto instance() -> DBConn*;
    auto db() -> QSqlDatabase;

    auto db(const QString& dbName) -> QSqlDatabase;

    auto connectionName(const QString& dbName) -> QString;

    void setType(const QString& type);
    void setHost(const QString& host);
    void setLogin(const QString& login);
    void setPass(const QString& pass);
    void setDBName(const QString& db_name);
    void setPort(int port);
private:
    DBConn() = default;
    ~DBConn() = default;

    static std::atomic<DBConn*> m_instance;
    static QString m_type;
    static QString m_login;
    static QString m_pass;
    static QString m_db_name;
    static QString m_host;
    static std::atomic<int> m_port;
    static std::mutex m_mutex;
};

    ///@brief Выполнение кода в основном треде для UI задач
    void dispatchToMainThread(std::function<void()> callback);

}

namespace SharedPoolCommon {
/** Создание пула объектов любого типа */
template<typename T, typename Key, typename... Args>
static auto sharedObject(const Key& key, Args&&... args) {
    static QHash<Key, std::shared_ptr<T>> ObjectsCache {};
    if (!ObjectsCache.contains(key)) {
        ObjectsCache[key] = std::make_shared<T>(std::forward<Args...>(args...));
    }
    return ObjectsCache[key].get();
}
}

namespace DecorationCommon {
    ///@brief Метод получения иконки приложения
    auto appIcon() -> const QIcon&;
    auto waitLoader() -> const QString&;

    auto closeIcon() -> const QIcon&;
    auto addIcon() -> const QIcon&;
    auto filterIcon() -> const QIcon&;
    auto saveIcon() -> const QIcon&;
    auto refreshIcon() -> const QIcon&;
    auto folderIcon() -> const QIcon&;
    auto playIcon() -> const QIcon&;
}
