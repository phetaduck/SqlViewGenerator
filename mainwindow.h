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
class QSyntaxHighlighter;

template <typename T, typename K = QObject*>
using ObjectHash = std::unordered_map<K, std::shared_ptr<T>>;


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
    void saveSqlSlot();
    void runSqlSlot();
    void openSqlSlot();
	
    void insertRandomFaceId();
    void insertRandomSkudId();

    void processPipeMessage(const QByteArray& data);
    void processNewDBRecords(const QList<QSqlRecord>& records);
    void fileExpanded(const QByteArray& data);

    void processNetworkReply(QNetworkReply* reply);
    void watchFile();

    void processFaceID_JSON(const QJsonObject& jsonObject);
    void processSkudID_JSON(const QJsonObject& jsonObject);

protected:
    virtual void initFields();
    virtual void connectSignals();


private:
    Ui::MainWindow *ui;
    std::unordered_map<QObject*, std::shared_ptr<TableColumnView>> m_columnViews;
    ObjectHash<QSyntaxHighlighter> m_syntaxHighlighters;
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

    void send(const QList<Event> &eventList);
};
