QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    classifer_name_table.cpp \
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
    models/sqlfiltermodel.cpp \
    models/sqltablemodel.cpp \
    tablecolumnview.cpp \
    updateviews.cpp \
    utils/application.cpp \
    utils/classifier_definitions.cpp \
    utils/jsonconverters.cpp \
    utils/modelmanager.cpp \
    utils/settings.cpp \
    utils/sqlsharedutils.cpp \
    utils/sqlsyntaxhighlighter.cpp \
    utils/sqlviewgenerator.cpp \
    views/sqltableview.cpp \
    viewsettings.cpp \
    widgets/asyncsqlcombobox.cpp \
    widgets/classifier_list.cpp \
    widgets/filtersqlcombobox.cpp \
    widgets/sqlcombobox.cpp

HEADERS += \
    classifer_name_table.h \
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
    models/sqlfiltermodel.h \
    models/sqltablemodel.h \
    tablecolumnview.h \
    updateviews.h \
    utils/application.h \
    utils/classifier_definitions.h \
    utils/jsonconverters.h \
    utils/modelmanager.h \
    utils/settings.h \
    utils/sqlsharedutils.h \
    utils/sqlsyntaxhighlighter.h \
    utils/sqlviewgenerator.h \
    views/sqltableview.h \
    viewsettings.h \
    widgets/asyncsqlcombobox.h \
    widgets/classifier_list.h \
    widgets/filtersqlcombobox.h \
    widgets/sqlcombobox.h

FORMS += \
    classifer_name_table.ui \
    dialogs/authentificationdialog.ui \
    mainwindow.ui \
    tablecolumnview.ui \
    updateviews.ui \
    viewsettings.ui

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

