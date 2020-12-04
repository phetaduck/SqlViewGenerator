#include "classifer_name_table.h"
#include "ui_classifer_name_table.h"

Classifer_Name_Table::Classifer_Name_Table(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Classifer_Name_Table)
{
    ui->setupUi(this);
}

Classifer_Name_Table::~Classifer_Name_Table()
{
    delete ui;
}

void Classifer_Name_Table::setClassifierData(const QString& name,
                                             const Classifiers::SqlTableDescriptor& classifierDef,
                                             int position)
{
    ui->gb_classifier->setTitle(name);
    ui->le_table->setText(classifierDef.TableName);
    ui->sb_num->setValue(position);
}
