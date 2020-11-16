#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include <QLocalSocket>
#include <QLocalServer>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>

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
};

class TableColumnView;
class SqlTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void insertRandomFaceId();


private:
    Ui::MainWindow *ui;
    std::unordered_map<QObject*, std::shared_ptr<TableColumnView>> m_columnViews;
    std::unique_ptr<QLocalSocket> m_pipeConnection = nullptr;
    std::unique_ptr<QLocalServer> m_localServer = nullptr;
    SqlTableModel* m_relationModel = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;

    QString m_relationtable = "id_key_number";
    FaceIDCache m_faceIdCache;
    QFileSystemWatcher m_fsWatcher;
    qint64 m_watchedFileSize = 0;

    QSqlDatabase dbconn;

    void updateSqlScript(const QString& table);
    void send(const QList<Event> &eventList);
};
