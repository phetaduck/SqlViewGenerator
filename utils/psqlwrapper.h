#pragma once

#include <QProcess>
#include <memory>
#include <vector>
#include <unordered_map>

class AsyncSqlTableModel;

class PsqlWrapper
{
public:
    void saveSchema(const QString& fileName,
                    const QString& dbName,
                    const QStringList& schemas);
    void saveSchema(const QString& fileName,
                    const QString& dbName,
                    const QStringList& schemas,
                    std::function<void(int, QProcess::ExitStatus)> callback);
    void saveDataOnly(const QString& fileName,
                      const QString& dbname,
                      const QStringList& schemas,
                      const QStringList& tables);
    void saveDataOnly(const QString& fileName,
                      const QString& dbname,
                      const QStringList& schemas,
                      const QStringList& tables,
                      std::function<void(int, QProcess::ExitStatus)> callback);
    void saveInsertScript(const QString& fileName,
                          const QString& dbname,
                          const QStringList& tables,
                          const QString& filter,
                          std::function<void(int, QProcess::ExitStatus)> callback);

    void serializeModelsData(const QString& fileName,
                             const QString& dbname);
private:
    struct BackupJob {
        QString fileName;
        QString dbName;
        QStringList schemas;
        std::shared_ptr<QProcess> process;
    };

    std::unordered_map<std::shared_ptr<QProcess>, BackupJob> m_jobs;

    std::vector<std::shared_ptr<AsyncSqlTableModel>> m_finishedModels;
    std::vector<std::shared_ptr<AsyncSqlTableModel>> m_modelsInProgress;

};

