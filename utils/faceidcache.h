#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>

#include <set>

struct FaceID
{
    int userId;
    QString keyNumber;
    double temp;
    QDateTime eventTime;
    QString name;

    static
    auto FromJSON(const QJsonObject& jsonObject) -> FaceID {
        FaceID out;
        out.userId = jsonObject.value("emp_id").toInt();
        out.keyNumber = jsonObject.value("key_number").toString();
        QLocale cLocale{QLocale::Russian};
        auto jsonValueString = jsonObject.value("temperature").toString();
        out.temp = cLocale.toDouble(jsonValueString);
        out.eventTime = QDateTime::fromString(
                            jsonObject.value("punch_time").toString(),
                            "yyyy-MM-ddThh:mm:ss.zzz");
        out.name = jsonObject.value("emp_name").toString();
        return out;
    }
};

class FaceIDCache : public QObject
{
    Q_OBJECT
public:
    void addFaceIDEvent(const FaceID& faceID);
    auto findByKeyNumber(const QString& keyNumber) -> QHash<QDateTime, FaceID>::iterator;
    auto findClosestByTime(const QDateTime& value) -> QHash<QDateTime, FaceID>::iterator;
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

    inline
    auto operator == (const SkudID& other) const -> bool {
        return other.position == position;
    }

    inline
    auto operator () () const -> std::size_t {
        return std::hash<int>{}(position);
    }

    static
    auto FromJSON(const QJsonObject& jsonObject) -> SkudID {
        SkudID out;
        out.direction = jsonObject.value("direct").toInt();
        out.keyNumber = jsonObject.value("keyNumber").toString();
        out.event = jsonObject.value("event").toInt();
        out.eventDateTime = QDateTime::fromString(
                            jsonObject.value("eventDate").toString(),
                            "dd.MM.yyyy hh:mm:ss");
        out.name = jsonObject.value("username").toString();
        out.position = jsonObject.value("position").toInt();
        out.options = jsonObject.value("options").toString();
        return out;
    }
};

inline
auto operator < (const SkudID& first, const SkudID& second) -> bool {
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

