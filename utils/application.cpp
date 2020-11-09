#include "application.h"

Application::Application(int &argc, char **argv, int flags)
    : QApplication(argc, argv, flags)
{
    setOrganizationName("phetaduck");
    setApplicationName("SqlViewGenerator");
/*
    QFile file(":/styles/res/styles/style.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QString(file.readAll()).simplified();
        setStyleSheet(styleSheet);
    }
    file.close();
*/

}
