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
    Classifiers::LoadClassifiers(filename);
    const auto& classifiers = Classifiers::GetClassifiers();
    ui->gb_Old->setClassifierList(classifiers);
}

void UpdateViews::saveJsonSlot()
{
    auto settings = Application::app()->settings();
    auto filename = QFileDialog::getSaveFileName(
                        this,
                        "Save classifier list",
                        settings.getClassifiersPath(),
                        "JSON *.json");
    Classifiers::SaveClassifiers(newClassifierList, filename);
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
    auto classifiers = Classifiers::GetClassifiers();
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

                tts << SqlViewGenerator::viewSql(sqlSettings);
                tts << SqlViewGenerator::insetFunctionSql(sqlSettings);
                tts << SqlViewGenerator::updateFunctionSql(sqlSettings);
                tts << SqlViewGenerator::deleteFunctionSql(sqlSettings);
                tts << SqlViewGenerator::insetTriggerSql(sqlSettings);
                tts << SqlViewGenerator::updateTriggerSql(sqlSettings);
                tts << SqlViewGenerator::deleteTriggerSql(sqlSettings);
            }
        }

        ui->pte_Sql->clear();
        ui->pte_Sql->appendPlainText(text);
    }

}
