#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>

#include <set>

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

struct SkudID
{
    QDateTime eventDateTime;
    int direction;
    int event;
    QString keyNumber;
    QString name;
    QString options;
    int position;

    bool operator == (const SkudID& other) {
        return other.position == position;
    }

    std::size_t operator () () {
        return std::hash<int>{}(position);
    }
};

inline
bool operator < (const SkudID& first, const SkudID& second) {
    return first.position < second.position;
}

class SkudIDCache : public QObject
{
    Q_OBJECT
public:
    void addSkudIDEvent(const SkudID& skudID);
    auto findByKeyNumber(const QString& keyNumber) -> std::set<SkudID>::iterator;
    auto cache() -> std::set<SkudID>&;
private:
    std::set<SkudID> m_cache;
};

