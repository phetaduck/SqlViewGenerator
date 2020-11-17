#include "filepolling.h"


void FilePolling::pollFile(const QString& path, int pollInterval) {
    auto it = std::find(m_paths.begin(),
                        m_paths.end(),
                        path);
    if (it != m_paths.end()) {
        return;
    }
    auto pollingLambda = [this](int pollInterval) {
        if (!m_pollTimer.isActive()) {
            connect(&m_pollTimer, &QTimer::timeout,
                    this, &FilePolling::pollFiles);
            m_pollTimer.setInterval(pollInterval);
            m_pollTimer.start();
        } else if (pollInterval != m_pollTimer.interval()) {
            m_pollTimer.setInterval(pollInterval);
        }
    };
    QFile file {path};
    if (file.open(QIODevice::ReadOnly)) {
        m_paths.push_back(path);
        m_fileSizes[path] = file.size();
        file.close();
        pollingLambda(pollInterval);
    } else {
        auto waitTimer = new QTimer(this);
        waitTimer->setInterval(500);
        connect(waitTimer, &QTimer::timeout,
                this, [this, waitTimer, path, pollInterval, pollingLambda] ()
        {
            QFile file {path};
            if (file.open(QIODevice::ReadOnly)) {
                m_paths.push_back(path);
                m_fileSizes[path] = file.size();
                file.close();
                pollingLambda(pollInterval);
                waitTimer->stop();
                waitTimer->deleteLater();
            }
        });
    }
}

void FilePolling::pollFile(const QString& path) {
    pollFile(path, 1000);
}

void FilePolling::removeFile(const QString& path) {
    auto it = std::find(m_paths.begin(),
                        m_paths.end(),
                        path);
    if (it != m_paths.end()) {
        m_paths.erase(it);
    }
    if (m_fileSizes.contains(path)) {
        m_fileSizes.remove(path);
    }
}

void FilePolling::pollFiles() {
    for (const auto& path: m_paths) {
        auto oldSize = m_fileSizes[path];
        QByteArray newData;
        QFile file {path};
        if (file.open(QIODevice::ReadOnly)) {
            auto size = file.size();
            if (size > oldSize) {
                file.seek(oldSize);
                newData = file.read(size - oldSize);
            }
            file.close();
            if (size > oldSize) {
                emit fileExpanded(path, newData, size, oldSize);
            } else if (size < oldSize) {
                emit fileShrank(path, size, oldSize);
            }
            m_fileSizes[path] = size;
        }
    }
}

auto FilePolling::paths() const -> const std::vector<QString>&
{
    return m_paths;
}

void FilePolling::clearPaths() {
    m_pollTimer.stop();
    m_paths.clear();
    m_fileSizes.clear();
}
