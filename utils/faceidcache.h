#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>

struct FaceID
{
    int userId;
    QString keyNumber;
    double temp;
    QDateTime eventTime;
    QString name;
};

class FaceIDCache : public QObject
{
    Q_OBJECT
public:
    void addFaceIDEvent(const FaceID& faceID);
    auto findByKeyNumber(const QString& keyNumber) -> QHash<QDateTime, FaceID>::iterator;
    auto cache() -> QHash<QDateTime, FaceID>&;
private:
    QHash<QDateTime, FaceID> m_cache;
};

