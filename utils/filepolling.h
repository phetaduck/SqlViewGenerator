#pragma once

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QHash>

class FilePolling : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    void pollFile(const QString& path, int pollInterval/** msecs **/);

    void pollFile(const QString& path /** 1000 msecs **/);

    void removeFile(const QString& path);

    auto paths() const -> const std::vector<QString>&;

    void clearPaths();

private slots:

    void pollFiles();


signals:
    void fileExpanded(const QString& filePath,
                      const QByteArray& newData,
                      qint64 newSize,
                      qint64 oldSize);
    void fileShrank(const QString& filePath, qint64 newSize, qint64 oldSize);
private:
    std::vector<QString> m_paths;
    QHash<QString, qint64> m_fileSizes;
    QTimer m_pollTimer;
};

