QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogs/authentificationdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    models/asyncsqltablemodel.cpp \
    models/searchablesqltablemodel.cpp \
    tablecolumnview.cpp \
    utils/application.cpp \
    utils/modelmanager.cpp \
    utils/settings.cpp \
    utils/sqlsharedutils.cpp \
    utils/sqlsyntaxhighlighter.cpp \
    widgets/asyncsqlcombobox.cpp \
    widgets/filtersqlcombobox.cpp \
    widgets/sqlcombobox.cpp

HEADERS += \
    dialogs/authentificationdialog.h \
    mainwindow.h \
    models/asyncsqltablemodel.h \
    models/searchablesqltablemodel.h \
    tablecolumnview.h \
    utils/application.h \
    utils/modelmanager.h \
    utils/settings.h \
    utils/sqlsharedutils.h \
    utils/sqlsyntaxhighlighter.h \
    widgets/asyncsqlcombobox.h \
    widgets/filtersqlcombobox.h \
    widgets/sqlcombobox.h

FORMS += \
    dialogs/authentificationdialog.ui \
    mainwindow.ui \
    tablecolumnview.ui

INCLUDEPATH += \
widgets \
utils \
model

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

DISTFILES += \
    libs/SqtlHighlighter/src/BUGS.txt \
    libs/SqtlHighlighter/src/CMakeLists.txt

