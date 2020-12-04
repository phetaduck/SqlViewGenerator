#ifndef POSTGRESQLDUMP_H
#define POSTGRESQLDUMP_H

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

#endif // POSTGRESQLDUMP_H
