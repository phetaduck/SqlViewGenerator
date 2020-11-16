#include "sqlsharedutils.h"

#include <QApplication>
#include <QTimer>
#include <QThread>
#include <QSqlError>
#include <QDebug>

std::atomic<ThreadingCommon::DBConn*> ThreadingCommon::DBConn::m_instance = nullptr;
QString ThreadingCommon::DBConn::m_login = "postgres";
QString ThreadingCommon::DBConn::m_pass = "postgres";
QString ThreadingCommon::DBConn::m_db_name = "postgres";
QString ThreadingCommon::DBConn::m_host = "localhost";
QString ThreadingCommon::DBConn::m_type = "QPSQL";
std::atomic<int> ThreadingCommon::DBConn::m_port = 5432;
std::mutex ThreadingCommon::DBConn::m_mutex = {};

void ThreadingCommon::dispatchToMainThread(std::function<void ()> callback)
{
    if (!qApp) return;
    QTimer* timer = new QTimer();
    ///@note таймер надо переместить в основной поток
    timer->moveToThread(qApp->thread());
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [=]()
    {
        ///@note Нужно понимать, что callback упадет,
        /// если ресурсы в нем уже не существуют
        callback();
        timer->deleteLater();
    });
    ///@note таймер запускаем в основном потоке
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
}

auto DecorationCommon::appIcon() -> const QIcon&
{
    static QIcon AppIcon {};
    return AppIcon;
}

auto DecorationCommon::addIcon() -> const QIcon&
{
    static QIcon out {":/img/res/add.png"};
    return out;
}

auto DecorationCommon::filterIcon() -> const QIcon&
{
    static QIcon out {":/img/res/filter.png"};
    return out;
}

auto DecorationCommon::saveIcon() -> const QIcon&
{
    static QIcon out {":/img/res/outline_save_black_24dp.png"};
    return out;
}

auto DecorationCommon::refreshIcon() -> const QIcon&
{
    static QIcon out {":/img/res/refresh.png"};
    return out;
}

auto DecorationCommon::folderIcon() -> const QIcon&
{
    static QIcon out {":/img/res/round_folder_open_black_24dp.png"};
    return out;
}

auto DecorationCommon::playIcon() -> const QIcon&
{
    static QIcon out {":/img/res/round_play_arrow_black_24dp.png"};
    return out;
}

auto DecorationCommon::waitLoader() -> const QString&
{
    static QString out = ":/img/res/ajax-loader.gif";
    return out;
}

auto DecorationCommon::closeIcon() -> const QIcon&
{
    static QIcon out {":/img/res/clear.png"};
    return out;
}

auto ThreadingCommon::DBConn::instance() -> DBConn*
{
    DBConn* p = m_instance.load(std::memory_order_acquire);
    if (p == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        p = m_instance.load(std::memory_order_relaxed);
        if (p == nullptr) {
            p = new DBConn();
            m_instance.store(p, std::memory_order_release);
        }
    }
    return p;
}

auto ThreadingCommon::DBConn::db() ->QSqlDatabase
{
    return db(m_db_name);
}

QSqlDatabase ThreadingCommon::DBConn::db(const QString& dbName)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto conname = connectionName(dbName);

    QSqlDatabase dbconn = QSqlDatabase::database(conname);
    if (!QSqlDatabase::contains(conname)) {
        dbconn = QSqlDatabase::addDatabase(m_type, conname);
        dbconn.setHostName(m_host);
        dbconn.setDatabaseName(dbName);
        dbconn.setUserName(m_login);
        dbconn.setPassword(m_pass);
        dbconn.setPort(m_port.load(std::memory_order_acquire));

        bool ok = dbconn.open();

        if (ok != true) {
            qCritical() << "ConnectionManager::open: con= " << conname
                         << ": Connection error=" << dbconn.lastError().text();
            return {};
        }
    }

    return dbconn;
}

QString ThreadingCommon::DBConn::connectionName(const QString& dbName)
{
    QThread* curThread = QThread::currentThread();

    return QString("CNM0x%1_%2_%3_%4_%5")
                      .arg((qlonglong)curThread, 0, 16)
                      .arg(m_host, dbName, m_login)
                      .arg(m_port);
}

void ThreadingCommon::DBConn::setType(const QString& type)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_type = type;
}

void ThreadingCommon::DBConn::setHost(const QString& host)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_host = host;
}

void ThreadingCommon::DBConn::setLogin(const QString& login)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_login = login;
}

void ThreadingCommon::DBConn::setPass(const QString& pass)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pass = pass;
}

void ThreadingCommon::DBConn::setDBName(const QString& db_name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_db_name = db_name;
}

void ThreadingCommon::DBConn::setPort(int port)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_port = port;
}
