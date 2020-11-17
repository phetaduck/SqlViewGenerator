#include "faceidcache.h"

#include <QTimer>

void FaceIDCache::addFaceIDEvent(const FaceID& faceID)
{
    m_cache.insert(faceID.eventTime, faceID);
    QTimer::singleShot(60000, this, [this, faceID]()
    {
        if (m_cache.contains(faceID.eventTime))
            m_cache.remove(faceID.eventTime);
    });
}

auto FaceIDCache::findByKeyNumber(const QString& keyNumber) -> QHash<QDateTime, FaceID>::iterator
{
    return std::find_if(
                m_cache.begin(),
                m_cache.end(),
                [&keyNumber](const FaceID& value)
    {
        return value.keyNumber == keyNumber;
    });
}

auto FaceIDCache::findClosestByTime(const QDateTime& value) -> QHash<QDateTime, FaceID>::iterator
{
    std::map<qint64, QDateTime> diffMap;
    for (const auto& faceID : m_cache) {
        auto timeDiff = std::abs(faceID.eventTime.toMSecsSinceEpoch() - value.toMSecsSinceEpoch());
        diffMap[timeDiff] = faceID.eventTime;
    }
    // std::map is sorted in ascending order by default
    return !diffMap.empty() ? m_cache.find(diffMap.begin()->second) : m_cache.end();
}

auto FaceIDCache::cache() -> QHash<QDateTime, FaceID>&
{
    return m_cache;
}

void SkudIDCache::addSkudIDEvent(const SkudID& skudID)
{
    m_cache.insert(skudID);
    QTimer::singleShot(60000, this, [this, skudID]()
    {
        if (m_cache.count(skudID))
            m_cache.erase(skudID);
    });
}

auto SkudIDCache::findByKeyNumber(const QString& keyNumber) -> std::set<SkudID>::iterator
{
    return std::find_if(
                m_cache.begin(),
                m_cache.end(),
                [&keyNumber](const SkudID& value)
    {
        return value.keyNumber == keyNumber;
    });
}

auto SkudIDCache::cache() -> std::set<SkudID>&
{
    return m_cache;
}
