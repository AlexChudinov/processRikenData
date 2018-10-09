#include "TimeEvents.h"

TimeEvents::TimeEvents(QObject *parent) : QObject(parent)
{
    setObjectName("TimeEvents");
}

TimeEvents::~TimeEvents()
{

}

void TimeEvents::blockingAddEvent(size_t start, size_t stop)
{
    Locker lock(mMutex);
    mTimeEvents.push_back({start, stop});
    Q_EMIT eventsUpdateNotify(mTimeEvents.size());
}

void TimeEvents::blockingAddProps(QVariantMap props)
{
    Locker lock(mMutex);
    mProps.reset(new QVariantMap(props));
    Q_EMIT propsUpdateNotify();
}
