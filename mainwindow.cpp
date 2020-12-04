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
#include <QFileDialog>
#include <QMessageBox>

#include "sqlsharedutils.h"
#include "application.h"
#include "sqlsyntaxhighlighter.h"
#include "models/sqlfiltermodel.h"
#include "updateviews.h"

#include "viewsettings.h"

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

void MainWindow::initTableSettings()
{
    auto settings = Application::app()->settings();

    const auto& dbMetaData = defaultDBs().value(settings.dbType());

    ui->viewSettings->viewSchemaCb()->setSqlModel(
                ModelManager::sharedSqlTableModel<AsyncSqlTableModel>(
                    dbMetaData.SchemaTable));
    ui->viewSettings->viewSchemaCb()->setSqlRelation(QSqlRelation{dbMetaData.SchemaTable,
                                                                  dbMetaData.SchemaTableNameColumn,
                                                                  dbMetaData.SchemaTableNameColumn});

}

void MainWindow::connectTableSettingsSignals()
{
    connect(ui->viewSettings, &ViewSettings::sqlSettingsChanged,
            this, [this](const SqlSettings& newSqlSettings)
    {
        if (ui->lv_Tables->currentIndex().isValid()) {
            auto sqlSettings = newSqlSettings;
            sqlSettings.table = ui->lv_Tables->currentIndex().data().toString();
            sqlSettings.tableSchema = ui->cb_TableSchema->sqlComboBox()->currentText();
            saveSqlSettings(sqlSettings.table, sqlSettings);
            updateSqlScript(sqlSettings.table);
        }
    });
}

void MainWindow::setCurrentDb(const QString& text)
{
    auto settings = Application::app()->settings();
    settings.setDbName(text);
    auto mgr = ThreadingCommon::DBConn::instance();
    mgr->setDBName(text);
    const auto& dbMetaData = defaultDBs().value(settings.dbType());
    m_schemaModel->setFilterAndSelectIfNeeded(dbMetaData.SchemaDB
                                              + " = '"
                                              + text
                                              + "'");
}

#include "psqlwrapper.h"

void MainWindow::saveSchema()
{
    auto directoryPath = QFileDialog::getExistingDirectory(this, "Save SCHEMA as",
                                                      Application::app()->settings().lastSqlFile());
    if (directoryPath.isEmpty()) return;
    auto writeSqlLambda = [&directoryPath, this]() {
        auto psqlWrapper = SharedPoolCommon::sharedObject<PsqlWrapper>(QString{"PsqlWrapper"});
        psqlWrapper->saveSchema(directoryPath
                                + QDir::separator()
                                + ui->cb_Databases->sqlComboBox()->currentText()
                                + "_schema.sql",
                                ui->cb_Databases->sqlComboBox()->currentText(),
                                {"public", "catalogs", "project_facility", "project_facility_profile"});
        QFile file {":/res/All_Classifiers.txt"};
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QStringList tables;
            QTextStream ts{&file};
            while (!ts.atEnd()) {
                QString line = ts.readLine();
                if (!line.isEmpty()) {
                    tables << line;
                }
            }
            file.close();
            psqlWrapper->saveDataOnly(directoryPath
                                      + QDir::separator()
                                      + ui->cb_Databases->sqlComboBox()->currentText()
                                      + "_classifiers.sql",
                                      ui->cb_Databases->sqlComboBox()->currentText(),
                                      {"public", "catalogs", "project_facility", "project_facility_profile"},
                                      tables);
        }
        {
            psqlWrapper->saveDataOnly(directoryPath
                                      + QDir::separator()
                                      + ui->cb_Databases->sqlComboBox()->currentText()
                                      + "_data.sql",
                                      ui->cb_Databases->sqlComboBox()->currentText(),
                                      {"public", "catalogs", "project_facility", "project_facility_profile"},
                                      {});
        }
    };
    if (QFileInfo::exists(directoryPath)) {
        QMessageBox overwriteDialog;
        overwriteDialog.setText("File already exists! Do you wish to overwrite it?");
        connect(&overwriteDialog, &QMessageBox::accepted,
                this, writeSqlLambda);
        overwriteDialog.exec();
    } else {
        writeSqlLambda();
    }
}

void MainWindow::initFields()
{

    auto settings = Application::app()->settings();

    ui->cb_Databases->setSqlRelation(defaultRelation(settings.dbType()));
    ui->cb_Databases->setData(settings.dbName());

    const auto& dbMetaData = defaultDBs().value(settings.dbType());

    m_schemaModel = ModelManager::sharedSqlTableModel<AsyncSqlTableModel>(
                        dbMetaData.SchemaTable);
    m_schemaModel->setFilter(dbMetaData.InfoDB + " = " + settings.dbName());

    m_schemaTablesModel = ModelManager::sharedSqlTableModel<AsyncSqlTableModel>(
                              dbMetaData.InfoTable);

    ui->cb_TableSchema->setSqlModel(
                ModelManager::sharedSqlTableModel<AsyncSqlTableModel>(
                    dbMetaData.SchemaTable));
    ui->cb_TableSchema->setSqlRelation(QSqlRelation{dbMetaData.SchemaTable,
                                        dbMetaData.SchemaTableNameColumn,
                                        dbMetaData.SchemaTableNameColumn});
    ui->cb_TableSchema->setData("public");

    ui->lv_Tables->setModel(m_schemaTablesModel);
    ui->lv_Tables->setModelColumn(
                m_schemaTablesModel->fieldIndex(
                    dbMetaData.InfoTableName));

    auto makeHighlighter = [this](auto e) {
        m_syntaxHighlighters[e] = std::make_shared<SQLSyntaxHighlighter>(
                                      e->document());
    };

    makeHighlighter(ui->pte_Commands);
    makeHighlighter(ui->te_Output);

    ui->pte_Commands->setPlainText(settings.lastCommands());

    initTableSettings();

}

SqlSettings MainWindow::updateSqlSettings(const QString& table)
{
    auto sqlSettings = getSqlSettings(table);
    sqlSettings.tableSchema = ui->cb_TableSchema->sqlComboBox()->currentText();
    sqlSettings.fieldOrder.clear();
    sqlSettings.fieldOrder.push_back(sqlSettings.primaryKey);
    sqlSettings.record = ThreadingCommon::DBConn::instance()->db().record(table);

    for (int i = 0; i < sqlSettings.record.count(); i++) {
        auto field = sqlSettings.record.field(i);
        if (field.name() != sqlSettings.primaryKey) {
            sqlSettings.fieldOrder.push_back(field.name());
        }
    }
    saveSqlSettings(table, sqlSettings);
    return sqlSettings;
}

void MainWindow::connectSignals()
{
    connect(ui->tb_SaveSchema, &QToolButton::clicked,
            this, &MainWindow::saveSchema);
    connectActionToPageCreation<UpdateViews>(ui->actionUpdate_Views);
    connect(ui->tb_SaveSql, &QToolButton::clicked,
            this, &MainWindow::saveSqlSlot);

    connect(ui->tb_RunCommands, &QToolButton::clicked,
            this, &MainWindow::runSqlSlot);

    connect(ui->tb_OpenSql, &QToolButton::clicked,
            this, &MainWindow::openSqlSlot);

    connect(ui->cb_Databases->sqlComboBox(), QOverload<int>::of(&SqlComboBox::currentIndexChanged),
            this, [this](int index)
    {
        auto settings = Application::app()->settings();
        auto dbName = ui->cb_Databases->sqlComboBox()->itemText(index);
        if (settings.dbName() != dbName) {
            setCurrentDb(dbName);
        }
    });

    connect(ui->cb_TableSchema->comboBox(), QOverload<int>::of(&SqlComboBox::currentIndexChanged),
            this, [this](int index)
    {
        QString schemaName =  ui->cb_TableSchema->sqlComboBox()->itemText(index);
        auto settings = Application::app()->settings();
        const auto& dbMetaData = defaultDBs().value(settings.dbType());
        m_schemaTablesModel->setFilterAndSelectIfNeeded(dbMetaData.InfoDB
                                                        + " = '"
                                                        + settings.dbName()
                                                        + "' AND "
                                                        + dbMetaData.InfoSchemaName
                                                        + " = '"
                                                        + schemaName
                                                        + "'"
                                                        + " AND "
                                                        + dbMetaData.InfoTableType
                                                        + " = "
                                                        + "'BASE TABLE'");
    });

    connect(ui->lv_Tables->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&)
    {
        auto table = current.data().toString();
        TableViewSettings tableViewSettings;
        tableViewSettings.TableName = table;
        ui->tv_SelectedTableContents->init(tableViewSettings, dbconn);

        auto sqlSettings = updateSqlSettings(table);

        ui->viewSettings->setSqlSettings(sqlSettings);
        updateSqlScript(table);
    });

    connectTableSettingsSignals();
}

void MainWindow::updateSqlScript(const QString& table)
{
    if (table.isEmpty()) return;

    auto settings = Application::app()->settings();
    auto sqlSettings = getSqlSettings(table);

    sqlSettings.table = table;

    sqlSettings.record = ui->tv_SelectedTableContents->sqlModel()->record();

    QString viewText = SqlViewGenerator::viewSql(sqlSettings);
    QString insertFunctionText = SqlViewGenerator::insetFunctionSql(sqlSettings);
    QString updateFunctionText = SqlViewGenerator::updateFunctionSql(sqlSettings);
    QString deleteFunctionText = SqlViewGenerator::deleteFunctionSql(sqlSettings);
    QString iTrgText = SqlViewGenerator::insetTriggerSql(sqlSettings);
    QString uTrgText = SqlViewGenerator::updateTriggerSql(sqlSettings);
    QString dTrgText = SqlViewGenerator::deleteTriggerSql(sqlSettings);

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

QString MainWindow::sqlSettingKey(const QString& table)
{
    auto settings = Application::app()->settings();
    return settings.dbType() + settings.dbName() + table;
}

auto MainWindow::getSqlSettings(const QString& table) -> SqlSettings
{
    auto settings = Application::app()->settings();
    auto settingsKey = settings.dbType() + settings.dbName() + table;
    return settings.tableSettings(settingsKey);
}

void MainWindow::saveSqlSettings(const QString& table, const SqlSettings& sqlSettings)
{
    auto settings = Application::app()->settings();
    auto settingsKey = settings.dbType() + settings.dbName() + table;
    settings.setTableSettings(settingsKey, sqlSettings);
}

