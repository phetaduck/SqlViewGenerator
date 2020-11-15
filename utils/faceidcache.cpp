#include "faceidcache.h"

#include <QTimer>

void FaceIDCache::addFaceIDEvent(const FaceID& faceID)
{
    m_cache.insert(faceID.eventTime, faceID);
    QTimer::singleShot(60000, this, [this, faceID]()
    {
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

auto FaceIDCache::cache() -> QHash<QDateTime, FaceID>&
{
    return m_cache;
}
