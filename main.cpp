#include "mainwindow.h"
#include "dialogs/authentificationdialog.h"
#include "application.h"
#include <QMessageBox>
#include <QSqlError>

enum Authorized {
    OK,
    Cancel,
    Error
};

Authorized authorization()
{
    AuthentificationDialog authDlg;
    if (!authDlg.exec())
        return Cancel;

    auto settings = Application::app()->settings();
    auto *mgr = ThreadingCommon::DBConn::instance();
    auto db = mgr->db();

    auto login = authDlg.login();
    auto password = authDlg.password();
    auto host = authDlg.host();
    auto port = authDlg.port();
    auto db_type = authDlg.dbType();

    mgr->setType(db_type);
    mgr->setHost(host);
    mgr->setDBName(defaultDB(db_type));
    mgr->setLogin(login);
    mgr->setPass(password);
    mgr->setPort(port);

    settings.setDbHostName(host);
    settings.setDbPort(port);
    settings.setLastLogin(login);
    settings.setDbPass(password);
    settings.setDBType(db_type);

    db = mgr->db();

    if (!db.isOpen()) {
        QMessageBox::warning(nullptr, QStringLiteral("Database error"), db.lastError().text());
        return Error;
    }
    return OK;
}

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    a.setStyleSheet(":/res/material-blue.qss");
    int result = 0;
    bool restart;
    do {
        restart = false;
        Authorized auth = authorization();
        if (auth == Error) {
            restart = true;
            continue;
        } else if (auth == Cancel)
            return result;
        MainWindow w;
        w.show();
        result = a.exec();
    }
    while (restart);
    return result;
}
