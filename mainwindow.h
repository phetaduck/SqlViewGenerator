#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include <QLocalSocket>
#include <QLocalServer>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>
#include <filepolling.h>

#include <memory>
#include <unordered_map>

#include "faceidcache.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Event {
    double latitude;
    double longitude;
    qint64 occurred;
    QString incident_type;
    QString id;
    qint64 system_id;
    QString reason;
};

class TableColumnView;
class SqlTableModel;
class SQLSyntaxHighlighter;
class AutoSqlTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QJsonArray jsonFromRecordList(const QList<QSqlRecord>& records);

    void highTempMsg(const FaceID& faceId, const SkudID& skudId);
    void keyNumMismatchMsg(const SkudID& skudId);

public slots:
    void insertRandomFaceId();
    void insertRandomSkudId();

    void processPipeMessage(const QByteArray& data);
    void processNewDBRecords(const QList<QSqlRecord>& records);
    void fileExpanded(const QByteArray& data);

    void processNetworkReply(QNetworkReply* reply);
    void watchFile();

    void processFaceID_JSON(const QJsonObject& jsonObject);
    void processSkudID_JSON(const QJsonObject& jsonObject);


private:
    Ui::MainWindow *ui;
    std::unordered_map<QObject*, std::shared_ptr<TableColumnView>> m_columnViews;
    std::unique_ptr<QLocalSocket> m_pipeConnection = nullptr;
    std::unique_ptr<QLocalServer> m_localServer = nullptr;
    SqlTableModel* m_relationModel = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;

    QString m_relationtable = "id_key_number";
    QString m_skudFilePath;

    FaceIDCache m_faceIdCache;
    SkudIDCache m_skudIdCache;
    FilePolling* m_fsWatcher = nullptr;

    QSqlDatabase dbconn;

    SQLSyntaxHighlighter* m_highlighter = nullptr;

    void send(const QList<Event> &eventList);
};
