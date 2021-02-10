#pragma once

#include <QWidget>

namespace Ui {
class PostgreSqlDump;
}

class PostgreSqlDump : public QWidget
{
    Q_OBJECT

public:
    explicit PostgreSqlDump(QWidget *parent = nullptr);
    ~PostgreSqlDump();

private:
    Ui::PostgreSqlDump *ui;
};
