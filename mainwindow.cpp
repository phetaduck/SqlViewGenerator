#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tablecolumnview.h"

#include <QTextStream>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>
#include <QDesktopWidget>
#include <QDateTime>
#include <QFileDialog>
#include <QNetworkReply>
#include <QFileDialog>
#include <QMessageBox>

#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

#include "sqlsharedutils.h"
#include "application.h"
#include "sqlsyntaxhighlighter.h"
#include "models/autosqltablemodel.h"

static int eventId = 0;

namespace ColumnsNS {

enum Columns {
    EventDate = 0,
    Direct = 1,
    Event = 2,
    KeyNumber = 3,
    Username = 4,
    Options = 5,
    Position = 6
};

}

template<typename PageClass>
void createPage(const QString& title) {
    auto newPage = new PageClass;
    newPage->setAttribute(Qt::WA_DeleteOnClose, true);
    QPoint center = Application::app()->desktop()->rect().center();
    center.setX(center.x() / 2);
    newPage->move(center - newPage->rect().center());
    newPage->setWindowTitle(title);
    newPage->show();
}

template<typename PageClass>
void connectActionToPageCreation(QAction* action) {
    QObject::connect(action, &QAction::triggered, [action] {
        createPage<PageClass>(action->text());
    });
}

template< typename T >
std::string int_to_hex( T i )
{
    std::stringstream stream;
    stream << "\\u"
           << std::setfill ('0') << std::setw(sizeof(T))
           << std::hex << i;
    return stream.str();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    std::srand(std::time(0));
    eventId = std::rand();
	
    initFields();
    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveSqlSlot()
{
    auto plainText = ui->pte_Commands->toPlainText();
    auto filePath = QFileDialog::getSaveFileName(this, "Save SQL as",
                                                 Application::app()->settings().lastSqlFile(),
                                                 "*.sql");
    if (filePath.isEmpty()) return;
    auto writeSqlLambda = [&filePath, &plainText]() {
        QFile file{filePath};
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream ts{&file};
            ts << plainText;
            file.close();
            Application::app()->settings().setLastSqlFile(filePath);
        } else {
            QMessageBox cannotWriteFileWarning;
            cannotWriteFileWarning.setText("Unable to write file.");
            cannotWriteFileWarning.exec();
        }
    };
    if (QFileInfo::exists(filePath)) {
        QMessageBox overwriteDialog;
        overwriteDialog.setText("File already exists! Do you wish to overwrite it?");
        connect(&overwriteDialog, &QMessageBox::accepted,
                this, writeSqlLambda);
        overwriteDialog.exec();
    } else {
        writeSqlLambda();
    }
}

void MainWindow::runSqlSlot()
{
    auto plainText = ui->pte_Commands->toPlainText();
    Application::app()->settings().setLastCommands(plainText);
    auto list = plainText.split(";");
    dbconn = ThreadingCommon::DBConn::instance()->db();
    for (const auto& command : list) {
        if (command.isEmpty()) continue;
        auto query = dbconn.exec(command);
        auto lastError = query.lastError().text();
        if (!lastError.isEmpty()) {
            ui->pte_Log->appendPlainText(lastError);
        } else {
            while (query.next()) {
                QJsonDocument jsonDoc{fromSqlRecord(query.record())};
                ui->pte_Log->appendPlainText(jsonDoc.toJson());
            }
        }
    }
}

void MainWindow::openSqlSlot()
{
    auto plainText = ui->pte_Commands->toPlainText();
    auto filePath = QFileDialog::getOpenFileName(this, "Open SQL file",
                                                 Application::app()->settings().lastSqlFile(),
                                                 tr("Sql (*.sql);;Text (*.txt);;Any (*.*)"));
    if (filePath.isEmpty()) return;
    QFile file{filePath};
    if (file.open(QIODevice::ReadOnly)) {
        ui->pte_Commands->setPlainText(file.readAll());
        file.close();
        Application::app()->settings().setLastSqlFile(filePath);
    } else {
        QMessageBox cannotWriteFileWarning;
        cannotWriteFileWarning.setText("Unable to open file.");
        cannotWriteFileWarning.exec();
    }
}

void MainWindow::initFields()
{
    auto settings = Application::app()->settings();

    m_networkManager = new QNetworkAccessManager();

    ui->pte_Log->appendPlainText(QLocale::system().name());

    ui->dsb_MaxTemp->setValue(settings.maxTemp());

    ui->le_Protocol->setText(settings.lastRemoteProtocol());
    ui->le_RemoteAddress->setText(settings.lastRemoteServer());
    ui->le_RemoteApi->setText(settings.lastRemoteAPI());

    ui->cb_Databases->setSqlRelation(defaultRelation(settings.dbType()));
    ui->cb_Databases->sqlComboBox()->setData(settings.dbName());

    auto makeHighlighter = [this](auto e) {
        m_syntaxHighlighters[e] = std::make_shared<SQLSyntaxHighlighter>(e->document());
    };

    makeHighlighter(ui->pte_Commands);
    makeHighlighter(ui->te_Output);

    ui->pte_Commands->setPlainText(settings.lastCommands());
    ui->le_pipeName->setText(settings.pipeName());
    ui->le_LocalServer->setText(settings.localServer());

    m_localServer = std::make_unique<QLocalServer>();
    m_pipeConnection = std::make_unique<QLocalSocket>();

    ui->le_WatchFile->setText(settings.watchFile());

    m_fsWatcher = new FilePolling (this);

}

void MainWindow::connectSignals()
{
    connect(ui->tb_SaveSql, &QToolButton::clicked,
            this, &MainWindow::saveSqlSlot);

    connect(ui->tb_RunCommands, &QToolButton::clicked,
            this, &MainWindow::runSqlSlot);

    connect(ui->tb_OpenSql, &QToolButton::clicked,
            this, &MainWindow::openSqlSlot);

    connect(ui->cb_Databases->sqlComboBox(), &SqlComboBox::currentTextChanged,
            this, [this](const QString& dbName)
    {
        auto settings = Application::app()->settings();
        if (settings.dbName() != dbName) {
            settings.setDbName(dbName);
            auto mgr = ThreadingCommon::DBConn::instance();
            mgr->setDBName(dbName);
            dbconn = ThreadingCommon::DBConn::instance()->db();
            if (!dbconn.isOpen()) return;
            auto tables = dbconn.tables();
            tables.sort();
            ui->listWidget->clear();
            ui->listWidget->addItems(tables);
        }
    });

    connect(ui->listWidget, &SqlListWidget::setAsRelation,
            this, [this](QString table)
    {
        ui->pte_Log->appendPlainText("Setting table: "
                                     + table
                                     + " as key number relation table");
        if (table.isEmpty()) m_relationModel = nullptr;
        ModelManager::sharedSqlTableModel(m_relationModel, std::move(table));
        m_relationModel->select();
    });

    connect(ui->listWidget->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&)
    {
        auto table = current.data().toString();
        ui->tv_SelectedTableContents->init({table}, dbconn);
    });

    connect(ui->listWidget, &SqlListWidget::addToWatchList,
            this, [this](QString table)
    {
        ui->pte_Log->appendPlainText("Adding table: "
                                     + table
                                     + " to watch list");
        auto model = ModelManager::sharedSqlTableModel<AutoSqlTableModel>(table);
        if (!model->isSelectedAtLeastOnce()) {
            model->select();
            connect(model, &AutoSqlTableModel::newRecords,
                    this, [this](const QList<QSqlRecord>& newRecords)
            {
                processNewDBRecords(newRecords);
            });
        }
    });

    connect(ui->tb_Refresh, &QToolButton::clicked,
            this, &MainWindow::insertRandomFaceId);

    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::processNetworkReply);

    connect(ui->dsb_MaxTemp, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [](double value)
    {
        auto settings = Application::app()->settings();
        if (value != settings.maxTemp()) {
            settings.setMaxTemp(value);
        }
    });

    connect(ui->tb_AddSkudID, &QToolButton::clicked,
            this, &MainWindow::insertRandomSkudId);

    connect(m_fsWatcher, &FilePolling::fileExpanded,
            this, [this](const QString& filePath, const QByteArray& newData, qint64, qint64)
    {
        ui->pte_Log->appendPlainText("File: "
                                     + filePath
                                     + " changed");
        ui->pte_Log->appendPlainText("New data: "
                                     + newData);
        fileExpanded(newData);
    });

    connect(ui->tb_OpenWatchFile, &QToolButton::clicked,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        settings.setWatchFile(QFileDialog::getOpenFileName(this,
                                                           "Choose SKUD file to watch",
                                                           settings.watchFile(), "*.csv"));
        ui->le_WatchFile->setText(settings.watchFile());
    });

    connect(ui->pb_WatchFile, &QPushButton::clicked,
            this, &MainWindow::watchFile);

    connect(m_localServer.get(), &QLocalServer::newConnection,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        auto pendingConnection = m_localServer->nextPendingConnection();
        ui->pte_Log->appendPlainText(settings.localServer() + ": new connection");
        if (pendingConnection) {
            connect(pendingConnection, &QLocalSocket::readyRead,
                    this, [this, pendingConnection]()
            {
                auto buffer = pendingConnection->readAll();
                processPipeMessage(buffer);
            });
        }
    });

    connect(ui->pb_CreateServer, &QPushButton::clicked,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        settings.setLocalServer(ui->le_LocalServer->text());

        if (m_localServer->listen(settings.localServer())) {
            ui->pte_Log->appendPlainText("Created local server: "
                                         + settings.localServer());
        } else {
            ui->pte_Log->appendPlainText("Failed to create local server: "
                                         + settings.localServer());
        }
    });

    connect(ui->pb_connectPipe, &QPushButton::clicked,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        settings.setPipeName(ui->le_pipeName->text());

        m_pipeConnection->connectToServer(settings.pipeName(),
                                          QIODevice::WriteOnly);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::highTempMsg(const FaceID& faceId, const SkudID& skudId)
{
    auto empName = skudId.name;
    if (empName.isEmpty()) {
        empName = faceId.name;
    }
    ui->pte_Log->appendPlainText("Temperature is above threshold, sending event...");
    Event event;
    event.id = QString::number(eventId++);
    event.latitude = 55.927916;
    event.longitude = 37.851449;
    event.occurred = skudId.eventDateTime.toSecsSinceEpoch();
    event.system_id = 17;
    event.incident_type = "26.2";
    event.reason = "У сотрудника ("
                   + empName
                   + ") повышенная температура тела ("
                   + QString::number(faceId.temp, 'f', 2)
                   + " С)";
    send({event});

}

void MainWindow::keyNumMismatchMsg(const SkudID& skudId)
{
    auto empName = skudId.name;
    if (empName.isEmpty()) {
        empName = m_relationModel->findIndex(
                      "emp_name",
                      "key_number",
                      skudId.keyNumber).data().toString();
    }
    /// @note send message
    //
    ui->pte_Log->appendPlainText("Matching faceID still not found, sending message...");
    Event event;
    event.id = QString::number(eventId++);
    event.latitude = 55.927916;
    event.longitude = 37.851449;
    event.occurred = skudId.eventDateTime.toSecsSinceEpoch();
    event.system_id = 17;
    event.incident_type = "26.1";
    auto closestIt = m_faceIdCache.findClosestByTime(skudId.eventDateTime);
    if (closestIt != m_faceIdCache.cache().end()) {
        auto closestFaceId = *closestIt;
        event.reason = "Сотрудник ("
                       + closestFaceId.name
                       + ") прошел по чужому пропуску ("
                       + empName
                       + ")";
        m_faceIdCache.cache().erase(closestIt);
    } else {
        event.reason = "Неопознанные лица прошли по пропуску сотрудника ("
                       + empName
                       + ")";
        m_faceIdCache.cache().clear();
    }
    send({event});
}

void MainWindow::send(const QList<Event> &eventList)
{
    auto settings = Application::app()->settings();
    settings.setLastRemoteProtocol(ui->le_Protocol->text());
    settings.setLastRemoteServer(ui->le_RemoteAddress->text());
    settings.setLastRemoteAPI(ui->le_RemoteApi->text());
    QNetworkRequest request(QUrl(settings.lastRemoteProtocol()
                                 + settings.lastRemoteServer()
                                 + settings.lastRemoteAPI()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonArray array;
    for (auto &event : eventList) {
        QJsonObject obj;
        obj.insert("latitude", event.latitude);
        obj.insert("longitude", event.longitude);
        obj.insert("occurred", event.occurred);
        obj.insert("incident_type", event.incident_type);
        obj.insert("id", event.id);
        obj.insert("system_id", event.system_id);
        obj.insert("reason", event.reason);
        array.append(obj);
    }
    QJsonDocument doc(array);
    ui->pte_Log->appendPlainText("Sending request to " + request.url().toString());
    ui->pte_Log->appendPlainText("Request body :" + doc.toJson());
    m_networkManager->post(request, doc.toJson());
}

void MainWindow::insertRandomFaceId()
{
    static std::vector<std::pair<int, QString>> emps {
        {4, "Коптяев"},
        {10, "Кистанов"},
    };
    static std::vector<float> temps {
        36.6f,
        38.1f,
    };

    std::srand(std::time(0));

    auto rEmp = emps[std::rand() % emps.size()];
    auto temp = temps[std::rand() % temps.size()];
    auto now = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzz");

    auto request = QString {"INSERT INTO oa_attendance_punch"
                            "("
                            "create_company_id,"
                            "create_company_name,"
                            "created_datetime,"
                            "creator_id,"
                            "creator_name,"
                            "emp_id,"
                            "emp_name,"
                            "org_id,"
                            "org_name,"
                            "punch_time,"
                            "type,"
                            "result_type,"
                            "device_ip,"
                            "is_attendance,"
                            "device_id,"
                            "device_area,"
                            "temperature"
                            ") VALUES ("
                            "1,"
                            "'test',"
                            "'%1',"
                            "1,"
                            "'admin',"
                            "%2,"
                            "'%3',"
                            "2,"
                            "'проходная 1',"
                            "'%4',"
                            "0,"
                            "0,"
                            "27383227,"
                            "1,"
                            "27,"
                            "32,"
                            "%5"
                            ");"}
                   .arg(now) // created_datetime
                   .arg(rEmp.first) // emp_id
                   .arg(rEmp.second) // emp_name
                   .arg(now) // punch_time
                   .arg(temp); // temperature
    ui->pte_Log->appendPlainText(request);
    auto query = ThreadingCommon::DBConn::instance()->db().exec(request);
    ui->pte_Log->appendPlainText(query.lastError().text());

}

void MainWindow::insertRandomSkudId()
{
    static std::vector<std::pair<QString, QString>> emps {
        {"00 00 00 D7 9B 4A", "C800h"},
        {"00 00 00 D7 62 40", "D800h"}
    };
    QFile file {m_skudFilePath};
    while (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {};
    QString all = file.readAll();
    file.close();
    int pos = 0;
    auto it = std::find(all.rbegin(), all.rend(), ';');
    if (it != all.rend()) {
        auto next = std::find(it + 1, all.rend(), ';');
        QString number;
        while (next != it) {
            auto c = *--next;
            if (c != ';') {
                number += c;
            }
        }
        pos = number.toInt();
    }
    QByteArray dataToWrite;
    auto index = std::rand() % emps.size();
    auto emp = emps[index];
    auto currentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    while (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {};
    QTextStream ts{&file};
    ts << currentTime
       << ";0;3;"
       << emp.first
       <<";;"
      << emp.second
      << ";"
      << QString::number(++pos)
      << ";"
      << "\n";
    ts << currentTime
       << ";0;10;"
       << emp.first
       <<";;"
      << emp.second
      << ";"
      << QString::number(++pos)
      << ";"
      << "\n";
    //file.write(dataToWrite);
    file.close();
}

void MainWindow::processPipeMessage(const QByteArray& data)
{
    ui->pte_Log->appendPlainText("New message:");
    ui->pte_Log->appendPlainText(data);

    auto json = QJsonDocument::fromJson(data);
    auto jsonObject = json.object();
    auto source = jsonObject.find("source");
    if (source != jsonObject.end()
            && source->toString() == "faceid")
    {
        // faceid
        processFaceID_JSON(jsonObject);
    } else {
        // skud
        processSkudID_JSON(jsonObject);
    }
}

void MainWindow::processNewDBRecords(const QList<QSqlRecord>& records)
{
    QJsonObject jsonMsg;
    jsonMsg.insert("messageType", "ecall");
    static int i = 1;
    if (i > 1000) {
        i = 1;
    }
    QString num = QString::fromStdString(int_to_hex(i++));
    QString packectId {"source:faceid, pid:"};
    packectId += num;

    jsonMsg.insert("packetId", packectId);
    jsonMsg.insert("source", "faceid");
    jsonMsg.insert("records", jsonFromRecordList(records));
    QJsonDocument doc {jsonMsg};
    auto msg = doc.toJson();
    if (m_pipeConnection && m_pipeConnection->isValid()) {
        m_pipeConnection->write(msg);
        ui->pte_Log->appendPlainText("Message successfuly sent");
    }
}

void MainWindow::fileExpanded(const QByteArray& data)
{
    QTextStream in(data);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList values = line.split(';');
        if (values.size() <= ColumnsNS::Position) {
            ui->pte_Log->appendPlainText("missing columnd in csv file");
            continue;
        }
        if (values.at(ColumnsNS::Event) == "3") continue;
        QJsonObject obj;
        obj.insert("eventDate", values.at(ColumnsNS::EventDate));
        obj.insert("direct", values.at(ColumnsNS::Direct));
        obj.insert("event", values.at(ColumnsNS::Event));
        obj.insert("keyNumber", values.at(ColumnsNS::KeyNumber));
        obj.insert("username", values.at(ColumnsNS::Username));
        obj.insert("options", values.at(ColumnsNS::Options));
        obj.insert("position", values.at(ColumnsNS::Position));
        QJsonDocument doc;
        doc.setObject(obj);
        if (m_pipeConnection->isValid()) {
            m_pipeConnection->write(doc.toJson());
            //ui->pte_Log->appendPlainText("New lines: ");
            //ui->pte_Log->appendPlainText(doc.toJson());
        } else {

        }
    }
}

void MainWindow::processNetworkReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        ui->pte_Log->appendPlainText(reply->errorString());
    } else {
        ui->pte_Log->appendPlainText("successfully created event");
    }
    reply->deleteLater();
}

void MainWindow::watchFile()
{
    auto filePath = ui->le_WatchFile->text();
    QFileInfo file {filePath};
    if (file.exists()) {
        m_skudFilePath = filePath;
        ui->pte_Log->appendPlainText("File: " + filePath + " exists");
        m_fsWatcher->clearPaths();
        m_fsWatcher->pollFile(filePath);
        ui->pte_Log->appendPlainText("Watching file: " + filePath);
    } else {
        ui->pte_Log->appendPlainText("Cannot open file: " + filePath);
    }
}

void MainWindow::processFaceID_JSON(const QJsonObject& jsonObject)
{
    auto faceIdJsonArray = jsonObject.value("records").toArray();
    for (const auto& jsonValue : faceIdJsonArray) {
        auto faceIdJson = jsonValue.toObject();
        auto newFaceID = FaceID::FromJSON(faceIdJson);
        ui->pte_Log->appendPlainText(faceIdJson.value("temperature").toString());
        QLocale cLocale{QLocale::Russian};
        auto jsonValueString = faceIdJson.value("temperature").toString();
        ui->pte_Log->appendPlainText(QString::number(
                    cLocale.toDouble(jsonValueString)));
        ui->pte_Log->appendPlainText(QString::number(
                    jsonValueString.toDouble()));
        ui->pte_Log->appendPlainText(QString::number(
                    faceIdJson.value("temperature").toDouble()));
        ui->pte_Log->appendPlainText("New FaceID temp: " + QString::number(newFaceID.temp, 'f', 2));
        m_faceIdCache.addFaceIDEvent(newFaceID);
    }
}

void MainWindow::processSkudID_JSON(const QJsonObject& jsonObject)
{
    auto newSkudID = SkudID::FromJSON(jsonObject);
    if (!newSkudID.keyNumber.isEmpty())
    {
        // find faceId event
        auto it = m_faceIdCache.findByKeyNumber(newSkudID.keyNumber);
        if (eventId == INT_MAX) eventId = std::rand();
        if (it != m_faceIdCache.cache().end()) {
            auto foundFaceID = *it;
            ui->pte_Log->appendPlainText("SKUD event. Key number: "
                                         + newSkudID.keyNumber
                                         + " event date "
                                         + newSkudID.eventDateTime.toString()
                                         + " name "
                                         + newSkudID.name
                                         + " temperature "
                                         + QString::number(foundFaceID.temp));
            ui->pte_Log->appendPlainText("Matching faceID found...");
            if (foundFaceID.temp > ui->dsb_MaxTemp->value()) {
                highTempMsg(foundFaceID, newSkudID);
            }
            m_faceIdCache.cache().erase(it);
            ui->pte_Log->appendPlainText("Removed faceID from cache");
            /// @note match found. high temp
        } else {
            /// @note match not found, wait 30 seconds
            ui->pte_Log->appendPlainText("Matching faceID not found, waiting 10 seconds...");
            QTimer::singleShot(10000, this, [this,
                               newSkudID]()
            {
                auto it = m_faceIdCache.findByKeyNumber(newSkudID.keyNumber);
                if (it != m_faceIdCache.cache().end()) {
                    auto foundFaceID = *it;
                    ui->pte_Log->appendPlainText("Matching faceID found...");
                    ui->pte_Log->appendPlainText("Temperature: "
                                                 + QString::number(foundFaceID.temp, 'f', 2));
                    if (foundFaceID.temp > ui->dsb_MaxTemp->value()) {
                        highTempMsg(foundFaceID, newSkudID);
                    }
                    m_faceIdCache.cache().erase(it);
                    ui->pte_Log->appendPlainText("Removed faceID from cache");
                    /// @note match found. high temp
                } else {
                    keyNumMismatchMsg(newSkudID);
                }
            });
        }
    }


}

QJsonArray MainWindow::jsonFromRecordList(const QList<QSqlRecord>& records)
{
    QJsonArray jsonArray;
    for (const auto& record : records) {
        // JSON
        QJsonObject jsonRecord;

        for (int i = 0; i < record.count(); i++) {
            auto fieldName = record.fieldName(i);
            auto fieldValue = record.value(i);

            // JSON
            jsonRecord.insert(fieldName, QJsonValue::fromVariant(fieldValue));
        }
        QString keyCard;
        if (m_relationModel
                && record.contains("emp_id")
                && m_relationModel->record().contains("id"))
        {
            keyCard = m_relationModel->findIndex(
                          "key_number",
                          "id",
                          record.value("emp_id")).data().toString();

        }
        if (!keyCard.isEmpty()) {
            jsonRecord.insert("key_number", keyCard);
        }
        jsonArray.push_back(jsonRecord);
    }
    return jsonArray;
}

