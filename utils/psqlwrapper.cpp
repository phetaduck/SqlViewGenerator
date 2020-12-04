#include "psqlwrapper.h"
#include "sqlsharedutils.h"
#include "application.h"
#include "models/asyncsqltablemodel.h"

#include <QFile>

void PsqlWrapper::saveSchema(const QString& fileName,
                             const QString& dbname,
                             const QStringList& schemas)
{
    saveSchema(fileName, dbname, schemas, [](int, QProcess::ExitStatus){});
}

void PsqlWrapper::saveSchema(const QString& fileName,
                             const QString& dbname,
                             const QStringList& schemas,
                             std::function<void(int, QProcess::ExitStatus)> callback)
{
    auto pgDumpProcess = std::make_shared<QProcess>();
    QStringList dump_input;
    auto settings = Application::app()->settings();

    dump_input << "-h" << settings.dbHostName();
    dump_input << "-f" << fileName;
    dump_input << "-F" << "p";
    for (const auto& schema : schemas) {
        dump_input << "-n" << schema;
    }
    dump_input << "--role=admin";
    dump_input << "--schema-only";
    //dump_input << "-U" << settings.lastLogin();
    //dump_input << "-d" << dbname;
    dump_input << QString{"--dbname=postgresql://%1:%2@%3:%4/%5"}
                  .arg(settings.lastLogin())
                  .arg(settings.dbPass())
                  .arg(settings.dbHostName())
                  .arg(settings.dbPort())
                  .arg(dbname);
    pgDumpProcess->connect(pgDumpProcess.get(), &QProcess::readyReadStandardOutput,
                     [pgDumpProcess]()
    {
        auto stdInput = QString::fromUtf8(pgDumpProcess->readAllStandardOutput());
        if (stdInput.contains("Password")) {
            auto settings = Application::app()->settings();
            pgDumpProcess->write(settings.dbPass().toUtf8());
        }
    });
    pgDumpProcess->connect(pgDumpProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [pgDumpProcess, callback](int code, QProcess::ExitStatus status)
    {
        if (callback) {
            callback(code, status);
        }
    });
    pgDumpProcess->connect(pgDumpProcess.get(), &QProcess::stateChanged,
                     [pgDumpProcess](QProcess::ProcessState newState)
    {
        qDebug() << newState;
    });
    BackupJob job;
    job.dbName = dbname;
    job.fileName = fileName;
    job.schemas = schemas;
    job.process = pgDumpProcess;
    m_jobs[pgDumpProcess] = job;
    pgDumpProcess->setProgram("pg_dump");
    pgDumpProcess->setArguments(dump_input);
    qDebug() << pgDumpProcess->program() + " " + pgDumpProcess->arguments().join(" ");
    pgDumpProcess->start();
}

void PsqlWrapper::saveDataOnly(const QString& fileName,
                               const QString& dbname,
                               const QStringList& schemas,
                               const QStringList& tables)
{
    saveDataOnly(fileName, dbname, schemas, tables, [](int, QProcess::ExitStatus){});
}

void PsqlWrapper::saveDataOnly(const QString& fileName,
                             const QString& dbname,
                             const QStringList& schemas,
                               const QStringList& tables,
                             std::function<void(int, QProcess::ExitStatus)> callback)
{
    auto pgDumpProcess = std::make_shared<QProcess>();
    QStringList dump_input;
    auto settings = Application::app()->settings();

    dump_input << "-h" << settings.dbHostName();
    dump_input << "-f" << fileName;
    dump_input << "-F" << "p";
    for (const auto& schema : schemas) {
        dump_input << "-n" << schema;
    }
    for (const auto& table : tables) {
        dump_input << "-t" << table;
    }
    dump_input << "--role=admin";
    dump_input << "--data-only";
    dump_input << "--column-inserts";
    //dump_input << "-U" << settings.lastLogin();
    //dump_input << "-d" << dbname;
    dump_input << QString{"--dbname=postgresql://%1:%2@%3:%4/%5"}
                  .arg(settings.lastLogin())
                  .arg(settings.dbPass())
                  .arg(settings.dbHostName())
                  .arg(settings.dbPort())
                  .arg(dbname);
    pgDumpProcess->connect(pgDumpProcess.get(), &QProcess::readyReadStandardOutput,
                     [pgDumpProcess]()
    {
        auto stdInput = QString::fromUtf8(pgDumpProcess->readAllStandardOutput());
        if (stdInput.contains("Password")) {
            auto settings = Application::app()->settings();
            pgDumpProcess->write(settings.dbPass().toUtf8());
        }
    });
    pgDumpProcess->connect(pgDumpProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [pgDumpProcess, callback](int code, QProcess::ExitStatus status)
    {
        if (callback) {
            callback(code, status);
        }
    });
    pgDumpProcess->connect(pgDumpProcess.get(), &QProcess::stateChanged,
                     [pgDumpProcess](QProcess::ProcessState newState)
    {
        qDebug() << newState;
    });
    BackupJob job;
    job.dbName = dbname;
    job.fileName = fileName;
    job.schemas = schemas;
    job.process = pgDumpProcess;
    m_jobs[pgDumpProcess] = job;
    pgDumpProcess->setProgram("pg_dump");
    pgDumpProcess->setArguments(dump_input);
    qDebug() << pgDumpProcess->program() + " " + pgDumpProcess->arguments().join(" ");
    pgDumpProcess->start();
}

void PsqlWrapper::saveInsertScript(const QString& fileName,
                                   const QString& dbname,
                                   const QStringList& tables,
                                   const QString& filter,
                                   std::function<void (int, QProcess::ExitStatus)> callback)
{

}

void PsqlWrapper::serializeModelsData(const QString& fileName,
                                      const QString& dbname)
{
    QFile insertScriptFile {fileName};
    if (insertScriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts{&insertScriptFile};
        for (const auto& model : m_finishedModels) {
            auto rowCount = model->rowCount();
            auto record = model->record();
            ts << "INSERT INTO " << model->tableName() << "\n";
            ts << "(" << "\n";
            ts << "\t";
            for (int i = 0; i < record.count(); i++) {
                if ( record.fieldName(i) != "deleted_at") {
                    ts << record.fieldName(i);
                }
                if (i < record.count() - 1
                        && !(i+1 == record.count() - 1
                             && record.fieldName(i+1) == "deleted_at"))
                {
                    ts << ",";
                }
                ts << "\n";
            }
            ts << "VALUES\n";
            for (int i = 0; i < rowCount; i++) {
                ts << "(";
                const auto& row = model->rowAt(i);

            }
        }
        insertScriptFile.close();
    }
}
