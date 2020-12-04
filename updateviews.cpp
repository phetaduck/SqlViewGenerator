#include "updateviews.h"
#include "ui_updateviews.h"

#include <QFileDialog>
#include <QLabel>

#include "utils/classifier_definitions.h"
#include "sqlsyntaxhighlighter.h"

UpdateViews::UpdateViews(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdateViews)
{
    ui->setupUi(this);
    connectSignals();
    new SQLSyntaxHighlighter(ui->pte_Sql->document());
}

UpdateViews::~UpdateViews()
{
    delete ui;
}

void UpdateViews::connectSignals()
{
    connect(ui->tb_OpenJson, &QToolButton::clicked,
            this, &UpdateViews::openJsonSlot);
    connect(ui->tb_SaveJson, &QToolButton::clicked,
            this, &UpdateViews::saveJsonSlot);
    connect(ui->tb_OpenCsv, &QToolButton::clicked,
            this, &UpdateViews::openCsvSlot);
    connect(ui->tb_SaveSql, &QToolButton::clicked,
            this, [this]()
    {
        auto settings = Application::app()->settings();
        auto prevDir = settings.value("file/prevSqlDir", QStringLiteral(".")).toString();
        auto dirname = QFileDialog::getExistingDirectory(this, "Save sql files to...",
                                                         prevDir);
        settings.setValue("file/prevSqlDir", dirname);
        for (auto it = m_sql.begin(); it != m_sql.end(); it++) {
            QFile file{dirname + QDir::separator() + it.key() + ".sql"};
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream ts{&file};
                ts << it.value();
                file.close();
            }
        }
    });
}

void UpdateViews::openJsonSlot()
{
    auto settings = Application::app()->settings();
    auto filename = QFileDialog::getOpenFileName(
                        this,
                        "Open classifier list",
                        settings.getClassifiersPath(),
                        "JSON *.json");
    settings.setClassifiersPath(filename);
    Classifiers::LoadSqlTableDescriptors(filename);
    const auto& classifiers = Classifiers::GetSqlTableDescriptors();
    ui->gb_Old->setClassifierList(classifiers);
    ui->te_OldDescriptors->clear();
    ui->te_OldTables->clear();
    QString text;
    QTextStream tts{&text};
    for (const auto& classifier: classifiers) {
        auto table = classifier.second.TableName;
        table.remove("catalogs.");
        table = "public." + table;
        auto dName = classifier.first;
        auto record = ThreadingCommon::DBConn::instance()->db("passport_show")
                      .record(table);
        if (record.contains("deleted_at")) {
            ui->te_OldDescriptors->append(dName);
            ui->te_OldTables->append(table);
        } else {
            ui->te_OldDescriptors->append("(" + dName + ") - missing");
            ui->te_OldTables->append("(" + table + ") - missing");
        }
    }

    ui->pte_Sql->clear();
}

void UpdateViews::saveJsonSlot()
{
    auto settings = Application::app()->settings();
    auto filename = QFileDialog::getSaveFileName(
                        this,
                        "Save classifier list",
                        settings.getClassifiersPath(),
                        "JSON *.json");
    const auto& classifiers = Classifiers::GetSqlTableDescriptors();
    Classifiers::SaveSqlTableDescriptors(classifiers, filename);
    //Classifiers::SaveClassifiers(newClassifierList, filename);
}

void UpdateViews::openCsvSlot()
{
    auto settings = Application::app()->settings();
    auto filename = QFileDialog::getOpenFileName(
                        this,
                        "Open csv file with new classifier list order",
                        settings.value("file/csv", QStringLiteral(".")).toString(),
                        "CSV *.CSV");
    settings.setValue("file/csv", filename);
    QFile file {filename};
    newClassifierList.clear();
    auto classifiers = Classifiers::GetSqlTableDescriptors();
    newClassifierList.reserve(classifiers.size());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts{&file};
        ts.setCodec("UTF-8");
        while (!ts.atEnd()) {
            QString name = ts.readLine();
            auto it = std::find_if(classifiers.begin(), classifiers.end(), [&](const auto& value)
            {
                return value.first == name;
            });

            if (it != classifiers.end()) {
                newClassifierList.push_back(*it);
                classifiers.erase(it);
            }
        }
        file.close();
        ui->pte_missing->clear();
        for (const auto& value : classifiers) {
            ui->pte_missing->appendPlainText(value.first);
            newClassifierList.push_back(value);
        }
        ui->gb_new->setClassifierList(newClassifierList);
        ui->pte_CList->clear();
        QString text;
        QTextStream tts{&text};
        for (const auto& classifier: newClassifierList) {
            auto record = ThreadingCommon::DBConn::instance()->db("passport_show").record(classifier.second.TableName);
            if (record.contains("deleted_at")) {
                ui->pte_CList->appendPlainText(classifier.second.TableName);
                SqlSettings sqlSettings;
                sqlSettings.table = classifier.second.TableName;
                sqlSettings.table.remove("catalogs.");
                m_sql[sqlSettings.table] = QString {};
                QTextStream lts{&m_sql[sqlSettings.table]};
                sqlSettings.tableSchema = "public";
                sqlSettings.viewSchema = "catalogs";
                sqlSettings.primaryKey = "id";
                sqlSettings.fieldOrder.clear();
                sqlSettings.fieldOrder.push_back(sqlSettings.primaryKey);
                sqlSettings.record = record;

                for (int i = 0; i < sqlSettings.record.count(); i++) {
                    auto field = sqlSettings.record.field(i);
                    if (field.name() != sqlSettings.primaryKey) {
                        sqlSettings.fieldOrder.push_back(field.name());
                    }
                }
                sqlSettings.sortColumn = "order_index";
                sqlSettings.sortEnabled = true;
                sqlSettings.dropFunctions = true;
                sqlSettings.dropView = false;
                sqlSettings.owner = "admin";

                lts << SqlViewGenerator::viewSql(sqlSettings);
                lts << SqlViewGenerator::insetFunctionSql(sqlSettings);
                lts << SqlViewGenerator::updateFunctionSql(sqlSettings);
                lts << SqlViewGenerator::deleteFunctionSql(sqlSettings);
                lts << SqlViewGenerator::insetTriggerSql(sqlSettings);
                lts << SqlViewGenerator::updateTriggerSql(sqlSettings);
                lts << SqlViewGenerator::deleteTriggerSql(sqlSettings);
                tts << m_sql[sqlSettings.table];
            }
        }

        ui->pte_Sql->clear();
        ui->pte_Sql->appendPlainText(text);
    }

}
