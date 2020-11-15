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
#include <QNetworkAccessManager>

#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

#include "sqlsharedutils.h"
#include "application.h"
#include "sqlsyntaxhighlighter.h"
#include "models/autosqltablemodel.h"

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

    auto settings = Application::app()->settings();

    ui->le_Protocol->setText(settings.lastRemoteProtocol());
    ui->le_RemoteAddress->setText(settings.lastRemoteServer());
    ui->le_RemoteApi->setText(settings.lastRemoteAPI());

    ui->cb_Databases->setSqlRelation(defaultRelation(settings.dbType()));
    ui->cb_Databases->sqlComboBox()->setData(settings.dbName());

    auto highlighter = new SQLSyntaxHighlighter(ui->pte_Commands->document());

    highlighter = new SQLSyntaxHighlighter(ui->te_Output->document());

    ui->pte_Commands->setPlainText(settings.lastCommands());
    ui->le_pipeName->setText(settings.pipeName());
    ui->le_LocalServer->setText(settings.localServer());

    m_localServer = std::make_unique<QLocalServer>();
    m_pipeConnection = std::make_unique<QLocalSocket>();

    connect(m_localServer.get(), &QLocalServer::newConnection,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        ui->pte_Log->appendPlainText(settings.localServer() + ": new connection");
        auto pendingConnection = m_localServer->nextPendingConnection();
        if (pendingConnection) {
            connect(pendingConnection, &QLocalSocket::readyRead,
                    this, [this, pendingConnection]()
            {
                auto buffer = pendingConnection->readAll();
                ui->pte_Log->appendPlainText("New message:");
                ui->pte_Log->appendPlainText(buffer);

                auto json = QJsonDocument::fromJson(buffer);
                auto jsonObject = json.object();
                auto source = jsonObject.find("source");
                if (source != jsonObject.end()
                        && source->toString() == "faceid")
                {
                    // faceid
                    auto oa_attendance_punch = jsonObject.find("oa_attendance_punch");
                    if (oa_attendance_punch != jsonObject.end()) {
                        auto faceIdJsonArray = jsonObject.value("oa_attendance_punch").toArray();
                        for (const auto& jsonValue : faceIdJsonArray) {
                            auto faceIdJson = jsonValue.toObject();
                            FaceID newFaceID {
                                faceIdJson.value("emp_id").toInt(),
                                        faceIdJson.value("key_number").toString(),
                                        faceIdJson.value("temperature").toString().toDouble(),
                                        QDateTime::fromString(
                                            faceIdJson.value("punch_time").toString(),
                                            "yyyy-MM-ddThh:mm:ss.zzz"),
                                        faceIdJson.value("emp_name").toString(),
                            };
                            m_faceIdCache.addFaceIDEvent(newFaceID);
                        }
                    }
                } else {
                    // skud
                    auto keyNumber = jsonObject.value("keyNumber").toString();
                    auto eventDate = QDateTime::fromString(
                                         jsonObject.value("eventDate").toString(),
                                         "dd.MM.yyyy hh:mm");
                    auto temp = jsonObject.value("temperature").toString().toDouble();
                    if (!keyNumber.isEmpty() &&
                            eventDate.isValid() )
                    {
                        // find faceId event
                        auto it = m_faceIdCache.findByKeyNumber(keyNumber);
                        static int eventId = 0;
                        if (eventId > 10000) eventId = 0;
                        if (it != m_faceIdCache.cache().end() && temp > 37.0) {
                            /// @note match found. high temp
                            Event event;
                            event.id = eventId++;
                            event.latitude = 0;
                            event.longitude = 0;
                            event.occurred = eventDate.toSecsSinceEpoch();
                            event.system_id = 17;
                            event.incident_type = "26.2";
                            send({event});
                        } else {
                            /// @note match not found, wait 30 seconds
                            QTimer::singleShot(30000, this, [this,
                                               keyNumber,
                                               eventDate]()
                            {
                                auto it = m_faceIdCache.findByKeyNumber(keyNumber);
                                if (it == m_faceIdCache.cache().end()) {
                                    /// @note send message
                                    //
                                    Event event;
                                    event.id = eventId++;
                                    event.latitude = 0;
                                    event.longitude = 0;
                                    event.occurred = eventDate.toSecsSinceEpoch();
                                    event.system_id = 17;
                                    event.incident_type = "26.1";
                                    send({event});
                                }
                            });
                        }
                    }
                }

            });
        }
    });

    connect(ui->pb_CreateServer, &QPushButton::clicked,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        settings.setLocalServer(ui->le_LocalServer->text());

        m_localServer->listen(settings.localServer());
    });

    connect(ui->pb_connectPipe, &QPushButton::clicked,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        settings.setPipeName(ui->le_pipeName->text());

        m_pipeConnection->connectToServer(settings.pipeName(),
                                          QIODevice::WriteOnly);
    });

    connect(ui->tb_RunCommands, &QToolButton::clicked,
            this, [this]()
    {
        auto plainText = ui->pte_Commands->toPlainText();
        Application::app()->settings().setLastCommands(plainText);
        auto list = plainText.split(";");
        dbconn = ThreadingCommon::DBConn::instance()->db();
        for (const auto& command : list) {
            auto query = dbconn.exec(command);
            auto lastError = query.lastError().text();
            if (!lastError.isEmpty()) {
                ui->pte_Log->appendPlainText(lastError);
            }
        }

    });

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
        if (table.isEmpty()) m_relationModel = nullptr;
        ModelManager::sharedSqlTableModel(m_relationModel, std::move(table));
        m_relationModel->select();
    });

    connect(ui->listWidget->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&)
    {
        auto table = current.data().toString();
        auto model = ModelManager::sharedSqlTableModel<SqlTableModel>(table);
        if (!model->isSelectedAtLeastOnce()) {
            model->select();
        }
        ui->tv_SelectedTableContents->setModel(model);
        updateSqlScript(table);
    });

    connect(ui->listWidget, &SqlListWidget::addToWatchList,
            this, [this](QString table)
    {
        auto model = ModelManager::sharedSqlTableModel<AutoSqlTableModel>(table);
        if (!model->isSelectedAtLeastOnce()) {
            model->select();
            connect(model, &AutoSqlTableModel::newRecords,
                    this, [this, model](QList<QSqlRecord> newRecords)
            {
                QJsonArray jsonArray;
                for (const auto& record : newRecords) {
                    // logging
                    QString logText;
                    QTextStream lts{&logText};
                    lts << "New row in "
                        << model->tableName()
                        << "\n\t";

                    // JSON
                    QJsonObject jsonRecord;

                    for (int i = 0; i < record.count(); i++) {
                        auto fieldName = record.fieldName(i);
                        auto fieldValue = record.value(i);

                        // JSON
                        jsonRecord.insert(fieldName, QJsonValue::fromVariant(fieldValue));

                        // LOGGING
                        lts << fieldName
                            << ": "
                            << fieldValue.toString()
                            << "\n\t";
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

                    // JSON
                    jsonArray.push_back(jsonRecord);
                    // LOGGING
                    ui->pte_Log->appendPlainText(logText);
                }
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
                jsonMsg.insert(model->tableName(), jsonArray);
                QJsonDocument doc {jsonMsg};
                auto msg = doc.toJson();
                if (m_pipeConnection && m_pipeConnection->isValid()) {
                    m_pipeConnection->write(msg);
                    ui->pte_Log->appendPlainText("Message successfuly sent");
                }

                // LOGGING
                ui->pte_Log->appendPlainText(msg);
            });
        }
    });

    connect(ui->tb_Refresh, &QToolButton::clicked,
            this, &MainWindow::insertRandomFaceId);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::send(const QList<Event> &eventList)
{
    auto settings = Application::app()->settings();
    settings.setLastRemoteProtocol(ui->le_Protocol->text());
    settings.setLastRemoteServer(ui->le_RemoteAddress->text());
    settings.setLastRemoteAPI(ui->le_RemoteApi->text());
    QNetworkAccessManager *manager = new QNetworkAccessManager();
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
        array.append(obj);
    }
    QJsonDocument doc(array);
    manager->post(request, doc.toJson());

    delete manager;
}

void MainWindow::updateSqlScript(const QString& table)
{
    auto record = dbconn.record(table);
    auto primaryKey = QString {"id"};

    QString viewSchema = "catalogs.";
    QString tableSchema = "public.";

    QString viewName = viewSchema + table;
    QString viewText;
    QTextStream vts (&viewText);
    vts << "--DROP VIEW IF EXISTS " << viewName << ";\n\n";
    vts << "CREATE OR REPLACE VIEW " << viewName << " AS\n" << "SELECT\n";

    QString insertFunctionText;
    QTextStream ifsText (&insertFunctionText);
    QString insertFunctionName = viewName + "_insert ()";

    ifsText << "--DROP FUNCTION IF EXISTS " << insertFunctionName << ";\n\n";
    ifsText << "CREATE OR REPLACE FUNCTION " << insertFunctionName << "\n";
    ifsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n" <<
               "DECLARE new_id integer;\n" <<
               "DECLARE new_row record;\n";
    ifsText << "BEGIN" << "\n";
    ifsText << "\t" << "INSERT INTO " << tableSchema << table << "\n\t(\n";

    QString updateFunctionText;
    QTextStream ufsText (&updateFunctionText);
    QString updateFunctionName = viewName + "_update ()";

    ufsText << "--DROP FUNCTION IF EXISTS " << updateFunctionName << ";\n\n";
    ufsText << "CREATE OR REPLACE FUNCTION " << updateFunctionName << "\n";
    ufsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n";

    if (record.contains("deleted_at")) {
        ufsText << "DECLARE deleted_timestamp timestamp with time zone;\n";
    }
    ufsText << "BEGIN" << "\n";
    if (record.contains("deleted_at")) {
        ufsText << "\t" << "IF NEW.deleted AND OLD.deleted THEN" << "\n" <<
                   "\t" << "\t" << "deleted_timestamp := (SELECT deleted_at FROM " << tableSchema <<
                   table << " WHERE " << primaryKey << " = OLD.id);" << "\n" <<
                   "\t" << "ELSIF NEW.deleted THEN" << "\n" <<
                   "\t" << "\t" << "deleted_timestamp := NOW();" << "\n" <<
                   "\t" << "ELSE" << "\n" <<
                   "\t" << "\t" << "deleted_timestamp := NULL;" << "\n" <<
                   "\t" << "END IF;" << "\n\n";
    }
    ufsText << "\t" << "UPDATE " << tableSchema << table <<
               "\n" << "\t" << " SET \n";

    QString deleteFunctionText;
    QTextStream dfsText (&deleteFunctionText);
    QString deleteFunctionName = viewName + "_delete ()";

    dfsText << "--DROP FUNCTION IF EXISTS " << deleteFunctionName << ";\n\n";
    dfsText << "CREATE OR REPLACE FUNCTION " << deleteFunctionName << "\n";
    dfsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n" <<
               "BEGIN" << "\n";

    if (record.contains("deleted_at")) {
        dfsText << "\t" << "UPDATE " << tableSchema << table << " SET deleted_at = NOW()" <<
                   " WHERE " << primaryKey << " = OLD." << primaryKey << ";\n";
    } else {
        dfsText << "\t" << "DELETE FROM " << tableSchema << table <<
                   " WHERE " << primaryKey << " = OLD." << primaryKey << ";\n";
    }
    dfsText << "\t" << "RETURN NULL;\n\n";
    dfsText << "END;\n" << "$BODY$;" << "\n\n";
    dfsText << "ALTER FUNCTION " << deleteFunctionName << "\n" <<
               "\t" << "OWNER TO admin;\n\n";

    for (int i = 0; i < record.count(); i++) {
        auto field = record.field(i);
        if (field.name() == "deleted_at") {
            vts << "\t" << "(" << table << "." << field.name() <<
                   " IS NOT NULL" << ")" << " AS deleted";
            if (i < record.count() - 1) {
                vts << ",";
            }
            vts << "\n";
        } else {
            vts << "\t" << table << "." << field.name();
            if (i < record.count() - 1) {
                vts << ",";
            }
            vts << "\n";
        }

        if (field.name() != primaryKey && field.name() != "deleted_at") {
            ifsText << "\t" << "\t" << field.name();
            if (i < record.count() - 1
                    && !(i+1 == record.count() - 1 && record.field(i+1).name() == "deleted_at")) {
                ifsText << ",";
            }
            ifsText << "\n";
        }

        if (field.name() != primaryKey) {
            ufsText << "\t" << "\t" << field.name() << " = ";
            if (field.name() == "deleted_at") {
                ufsText << "deleted_timestamp";
            } else {
                ufsText << "NEW." << field.name();
            }
            if (i < record.count() - 1) {
                ufsText << ",";
            }
            ufsText << "\n";
        }
    }

    vts << "FROM " << tableSchema << table << ";\n\n";

    ifsText << "\t" << ") VALUES (\n";

    for (int i = 0; i < record.count(); i++) {
        auto field = record.field(i);
        if (field.name() != primaryKey && field.name() != "deleted_at") {
            ifsText << "\t" << "\t" << "NEW." << field.name();
            if (i < record.count() - 1
                    && !(i+1 == record.count() - 1 && record.field(i+1).name() == "deleted_at")) {
                ifsText << ",";
            }
            ifsText << "\n";
        }
    }

    ifsText << "\t" << ")\n" << "\t" << "RETURNING " << primaryKey << " INTO new_id;\n\n";
    ifsText << "SELECT * INTO new_row FROM " << viewName << " WHERE " <<
               primaryKey << " = new_id;\n\n";
    ifsText << "RETURN new_row;\n\nEXCEPTION WHEN unique_violation THEN\n" <<
               "RAISE EXCEPTION 'record already exists';\n\n";

    ifsText << "END;\n" << "$BODY$;" << "\n\n";
    ifsText << "ALTER FUNCTION " << insertFunctionName << "\n" <<
               "\t" << "OWNER TO admin;\n\n";
    ufsText << "\t" << "\n";
    ufsText << "\t" << "WHERE  " << primaryKey << " = NEW." << primaryKey << ";\n\n";
    ufsText << "\t" << "RETURN NEW;\n\n";

    ufsText << "END;\n" << "$BODY$;" << "\n\n";
    ufsText << "ALTER FUNCTION " << updateFunctionName << "\n" <<
               "\t" << "OWNER TO admin;\n\n";

    QString iTrgText;
    QTextStream iTrgTS (&iTrgText);
    QString iTrgName = table + "_insert_trg";

    iTrgTS << "CREATE TRIGGER " << iTrgName << "\n";
    iTrgTS << "\t"  << "INSTEAD OF INSERT" << "\n";
    iTrgTS << "\t"  << "ON " << viewName << "\n";
    iTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    iTrgTS << "\t"  << "EXECUTE PROCEDURE " << insertFunctionName << ";" << "\n" << "\n";

    QString uTrgText;
    QTextStream uTrgTS (&uTrgText);
    QString uTrgName = table + "_update_trg";

    uTrgTS << "CREATE TRIGGER " << uTrgName << "\n";
    uTrgTS << "\t"  << "INSTEAD OF UPDATE" << "\n";
    uTrgTS << "\t"  << "ON " << viewName << "\n";
    uTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    uTrgTS << "\t"  << "EXECUTE PROCEDURE " << updateFunctionName << ";" << "\n" << "\n";

    QString dTrgText;
    QTextStream dTrgTS (&dTrgText);
    QString dTrgName = table + "_delete_trg";

    dTrgTS << "CREATE TRIGGER " << dTrgName << "\n";
    dTrgTS << "\t"  << "INSTEAD OF DELETE" << "\n";
    dTrgTS << "\t"  << "ON " << viewName << "\n";
    dTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    dTrgTS << "\t"  << "EXECUTE PROCEDURE " << deleteFunctionName << ";" << "\n" << "\n";

    ui->te_Output->setText(
                viewText
                + insertFunctionText
                + updateFunctionText
                + deleteFunctionText
                + iTrgText
                + uTrgText
                + dTrgText
                );
}

void MainWindow::insertRandomFaceId()
{
    static std::vector<std::pair<int, QString>> emps {
        {4, "Коптяев"},
        {10, "Кистанов"},
    };
    static std::vector<float> temps {
       36.6f,
       38.0f,
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

