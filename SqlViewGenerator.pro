QT       += core gui sql concurrent network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogs/authentificationdialog.cpp \
    item_delegates/baseitemdelegate.cpp \
    item_delegates/boolitemdelegate.cpp \
    item_delegates/comboboxitemdelegate.cpp \
    item_delegates/datetimeitemdelegate.cpp \
    item_delegates/floatformatteditemdelegate.cpp \
    item_delegates/htmlitemdelegate.cpp \
    item_delegates/intformatteditemdelegate.cpp \
    item_delegates/relationalitemdelegate.cpp \
    item_delegates/styleditemdelegate.cpp \
    main.cpp \
    mainwindow.cpp \
    models/asyncsqltablemodel.cpp \
    models/autosqltablemodel.cpp \
    models/sqltablemodel.cpp \
    tablecolumnview.cpp \
    utils/application.cpp \
    utils/faceidcache.cpp \
    utils/filepolling.cpp \
    utils/modelmanager.cpp \
    utils/settings.cpp \
    utils/sqlsharedutils.cpp \
    utils/sqlsyntaxhighlighter.cpp \
    views/sqltableview.cpp \
    widgets/asyncsqlcombobox.cpp \
    widgets/filtersqlcombobox.cpp \
    widgets/sqlcombobox.cpp \
    widgets/sqllistwidget.cpp

HEADERS += \
    dialogs/authentificationdialog.h \
    item_delegates/baseitemdelegate.h \
    item_delegates/boolitemdelegate.h \
    item_delegates/comboboxitemdelegate.h \
    item_delegates/datetimeitemdelegate.h \
    item_delegates/floatformatteditemdelegate.h \
    item_delegates/htmlitemdelegate.h \
    item_delegates/intformatteditemdelegate.h \
    item_delegates/relationalitemdelegate.h \
    item_delegates/styleditemdelegate.h \
    mainwindow.h \
    models/asyncsqltablemodel.h \
    models/autosqltablemodel.h \
    models/sqltablemodel.h \
    tablecolumnview.h \
    utils/application.h \
    utils/faceidcache.h \
    utils/filepolling.h \
    utils/modelmanager.h \
    utils/settings.h \
    utils/sqlsharedutils.h \
    utils/sqlsyntaxhighlighter.h \
    views/sqltableview.h \
    widgets/asyncsqlcombobox.h \
    widgets/filtersqlcombobox.h \
    widgets/sqlcombobox.h \
    widgets/sqllistwidget.h

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

