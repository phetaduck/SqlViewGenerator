#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tablecolumnview.h"

#include <QTextStream>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>
#include <QDesktopWidget>

#include "sqlsharedutils.h"
#include "application.h"
#include "sqlsyntaxhighlighter.h"

template<typename PageClass>
void createPage(const QString& title) {
    auto newPage = new PageClass;
    newPage->setAttribute(Qt::WA_DeleteOnClose, true);
    QPoint center = Application::app()->desktop()->rect().center();
    center.setX(center.x() / 2);
    newPage->move(center - newPage->rect().center());
    newPage->setWindowTitle(title);
    newPage->show();
}

template<typename PageClass>
void connectActionToPageCreation(QAction* action) {
    QObject::connect(action, &QAction::triggered, [action] {
        createPage<PageClass>(action->text());
    });
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto settings = Application::app()->settings();

    ui->cb_Databases->setSqlRelation(defaultRelation(settings.dbType()));
    ui->cb_Databases->sqlComboBox()->setData(settings.dbName());

    auto highlighter = new SQLSyntaxHighlighter(ui->pte_Commands->document());

    highlighter = new SQLSyntaxHighlighter(ui->te_Output->document());

    ui->pte_Commands->setPlainText(settings.lastCommands());

    connect(ui->tb_RunCommands, &QToolButton::clicked,
            this, [this]()
    {
        auto plainText = ui->pte_Commands->toPlainText();
        Application::app()->settings().setLastCommands(plainText);
        auto list = plainText.split(";");
        dbconn = ThreadingCommon::DBConn::instance()->db();
        for (const auto& command : list) {
            auto query = dbconn.exec(command);
            auto lastError = query.lastError().text();
            if (!lastError.isEmpty()) {
                ui->pte_Log->appendPlainText(lastError);
            }
        }

    });

    connect(ui->cb_Databases->sqlComboBox(), &SqlComboBox::currentTextChanged,
            this, [this](const QString& dbName)
    {
        auto settings = Application::app()->settings();
        if (settings.dbName() != dbName) {
            settings.setDbName(dbName);
            dbconn = ThreadingCommon::DBConn::instance()->db(dbName);
            if (!dbconn.isOpen()) return;
            auto tables = dbconn.tables();
            tables.sort();
            ui->listWidget->clear();
            ui->listWidget->addItems(tables);
        }
    });

    connect(ui->listWidget->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&)
    {
        auto table = current.data().toString();
        updateSqlScript(table);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateSqlScript(const QString& table)
{
    auto record = dbconn.record(table);
    auto primaryKey = QString {"id"};

    QString viewSchema = "catalogs.";
    QString tableSchema = "public.";

    QString viewName = viewSchema + table;
    QString viewText;
    QTextStream vts (&viewText);
    vts << "--DROP VIEW IF EXISTS " << viewName << ";\n\n";
    vts << "CREATE OR REPLACE VIEW " << viewName << " AS\n" << "SELECT\n";

    QString insertFunctionText;
    QTextStream ifsText (&insertFunctionText);
    QString insertFunctionName = viewName + "_insert ()";

    ifsText << "--DROP FUNCTION IF EXISTS " << insertFunctionName << ";\n\n";
    ifsText << "CREATE OR REPLACE FUNCTION " << insertFunctionName << "\n";
    ifsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n" <<
               "DECLARE new_id integer;\n" <<
               "DECLARE new_row record;\n";
    ifsText << "BEGIN" << "\n";
    ifsText << "\t" << "INSERT INTO " << tableSchema << table << "\n\t(\n";

    QString updateFunctionText;
    QTextStream ufsText (&updateFunctionText);
    QString updateFunctionName = viewName + "_update ()";

    ufsText << "--DROP FUNCTION IF EXISTS " << updateFunctionName << ";\n\n";
    ufsText << "CREATE OR REPLACE FUNCTION " << updateFunctionName << "\n";
    ufsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n";

    if (record.contains("deleted_at")) {
        ufsText << "DECLARE deleted_timestamp timestamp with time zone;\n";
    }
    ufsText << "BEGIN" << "\n";
    if (record.contains("deleted_at")) {
        ufsText << "\t" << "IF NEW.deleted AND OLD.deleted THEN" << "\n" <<
                   "\t" << "\t" << "deleted_timestamp := (SELECT deleted_at FROM " << tableSchema <<
                   table << " WHERE " << primaryKey << " = OLD.id);" << "\n" <<
                   "\t" << "ELSIF NEW.deleted THEN" << "\n" <<
                   "\t" << "\t" << "deleted_timestamp := NOW();" << "\n" <<
                   "\t" << "ELSE" << "\n" <<
                   "\t" << "\t" << "deleted_timestamp := NULL;" << "\n" <<
                   "\t" << "END IF;" << "\n\n";
    }
    ufsText << "\t" << "UPDATE " << tableSchema << table <<
               "\n" << "\t" << " SET \n";

    QString deleteFunctionText;
    QTextStream dfsText (&deleteFunctionText);
    QString deleteFunctionName = viewName + "_delete ()";

    dfsText << "--DROP FUNCTION IF EXISTS " << deleteFunctionName << ";\n\n";
    dfsText << "CREATE OR REPLACE FUNCTION " << deleteFunctionName << "\n";
    dfsText << "\t" << "RETURNS trigger" << "\n" <<
               "\t" << "LANGUAGE 'plpgsql'" << "\n" <<
               "\t" << "COST 100" << "\n" <<
               "\t" << "VOLATILE NOT LEAKPROOF SECURITY DEFINER" << "\n" <<
               "AS $BODY$" << "\n" <<
               "BEGIN" << "\n";

    if (record.contains("deleted_at")) {
        dfsText << "\t" << "UPDATE " << tableSchema << table << " SET deleted_at = NOW()" <<
                   " WHERE " << primaryKey << " = OLD." << primaryKey << ";\n";
    } else {
        dfsText << "\t" << "DELETE FROM " << tableSchema << table <<
                   " WHERE " << primaryKey << " = OLD." << primaryKey << ";\n";
    }
    dfsText << "\t" << "RETURN NULL;\n\n";
    dfsText << "END;\n" << "$BODY$;" << "\n\n";
    dfsText << "ALTER FUNCTION " << deleteFunctionName << "\n" <<
               "\t" << "OWNER TO admin;\n\n";

    for (int i = 0; i < record.count(); i++) {
        auto field = record.field(i);
        if (field.name() == "deleted_at") {
            vts << "\t" << "(" << table << "." << field.name() <<
                   " IS NOT NULL" << ")" << " AS deleted";
            if (i < record.count() - 1) {
                vts << ",";
            }
            vts << "\n";
        } else {
            vts << "\t" << table << "." << field.name();
            if (i < record.count() - 1) {
                vts << ",";
            }
            vts << "\n";
        }

        if (field.name() != primaryKey && field.name() != "deleted_at") {
            ifsText << "\t" << "\t" << field.name();
            if (i < record.count() - 1
                    && !(i+1 == record.count() - 1 && record.field(i+1).name() == "deleted_at")) {
                ifsText << ",";
            }
            ifsText << "\n";
        }

        if (field.name() != primaryKey) {
            ufsText << "\t" << "\t" << field.name() << " = ";
            if (field.name() == "deleted_at") {
                ufsText << "deleted_timestamp";
            } else {
                ufsText << "NEW." << field.name();
            }
            if (i < record.count() - 1) {
                ufsText << ",";
            }
            ufsText << "\n";
        }
    }

    vts << "FROM " << tableSchema << table << ";\n\n";

    ifsText << "\t" << ") VALUES (\n";

    for (int i = 0; i < record.count(); i++) {
        auto field = record.field(i);
        if (field.name() != primaryKey && field.name() != "deleted_at") {
            ifsText << "\t" << "\t" << "NEW." << field.name();
            if (i < record.count() - 1
                    && !(i+1 == record.count() - 1 && record.field(i+1).name() == "deleted_at")) {
                ifsText << ",";
            }
            ifsText << "\n";
        }
    }

    ifsText << "\t" << ")\n" << "\t" << "RETURNING " << primaryKey << " INTO new_id;\n\n";
    ifsText << "SELECT * INTO new_row FROM " << viewName << " WHERE " <<
               primaryKey << " = new_id;\n\n";
    ifsText << "RETURN new_row;\n\nEXCEPTION WHEN unique_violation THEN\n" <<
               "RAISE EXCEPTION 'record already exists';\n\n";

    ifsText << "END;\n" << "$BODY$;" << "\n\n";
    ifsText << "ALTER FUNCTION " << insertFunctionName << "\n" <<
               "\t" << "OWNER TO admin;\n\n";
    ufsText << "\t" << "\n";
    ufsText << "\t" << "WHERE  " << primaryKey << " = NEW." << primaryKey << ";\n\n";
    ufsText << "\t" << "RETURN NEW;\n\n";

    ufsText << "END;\n" << "$BODY$;" << "\n\n";
    ufsText << "ALTER FUNCTION " << updateFunctionName << "\n" <<
               "\t" << "OWNER TO admin;\n\n";

    QString iTrgText;
    QTextStream iTrgTS (&iTrgText);
    QString iTrgName = table + "_insert_trg";

    iTrgTS << "CREATE TRIGGER " << iTrgName << "\n";
    iTrgTS << "\t"  << "INSTEAD OF INSERT" << "\n";
    iTrgTS << "\t"  << "ON " << viewName << "\n";
    iTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    iTrgTS << "\t"  << "EXECUTE PROCEDURE " << insertFunctionName << ";" << "\n" << "\n";

    QString uTrgText;
    QTextStream uTrgTS (&uTrgText);
    QString uTrgName = table + "_update_trg";

    uTrgTS << "CREATE TRIGGER " << uTrgName << "\n";
    uTrgTS << "\t"  << "INSTEAD OF UPDATE" << "\n";
    uTrgTS << "\t"  << "ON " << viewName << "\n";
    uTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    uTrgTS << "\t"  << "EXECUTE PROCEDURE " << updateFunctionName << ";" << "\n" << "\n";

    QString dTrgText;
    QTextStream dTrgTS (&dTrgText);
    QString dTrgName = table + "_delete_trg";

    dTrgTS << "CREATE TRIGGER " << dTrgName << "\n";
    dTrgTS << "\t"  << "INSTEAD OF DELETE" << "\n";
    dTrgTS << "\t"  << "ON " << viewName << "\n";
    dTrgTS << "\t"  << "FOR EACH ROW" << "\n";
    dTrgTS << "\t"  << "EXECUTE PROCEDURE " << deleteFunctionName << ";" << "\n" << "\n";

    ui->te_Output->setText(
                viewText
                + insertFunctionText
                + updateFunctionText
                + deleteFunctionText
                + iTrgText
                + uTrgText
                + dTrgText
                );
}

