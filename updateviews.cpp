#include "updateviews.h"
#include "ui_updateviews.h"

#include <QFileDialog>
#include <QLabel>

#include "utils/classifier_definitions.h"

UpdateViews::UpdateViews(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdateViews)
{
    ui->setupUi(this);
    connectSignals();
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
    Classifiers::SaveClassifiers(filename);
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
    decltype (Classifiers::KnownClassifiers) newClassifierList;
    const auto& classifiers = Classifiers::GetClassifiers();
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
            }
        }
        file.close();
        ui->gb_new->setClassifierList(newClassifierList);
    }

}
