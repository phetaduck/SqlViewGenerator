#include "postgresqldump.h"
#include "ui_postgresqldump.h"

PostgreSqlDump::PostgreSqlDump(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PostgreSqlDump)
{
    ui->setupUi(this);
}

PostgreSqlDump::~PostgreSqlDump()
{
    delete ui;
}
