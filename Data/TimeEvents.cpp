#include "TimeEvents.h"

TimeEvents::TimeEvents(QObject *parent) : QObject(parent)
{
    setObjectName("TimeEvents");
}

TimeEvents::~TimeEvents()
{

}

void TimeEvents::blockingAddEvent(TimeEvent evt)
{
    Locker lock(mMutex);
    mTimeEvents.push_back(evt);
    Q_EMIT eventsUpdateNotify(static_cast<size_t>(mTimeEvents.size()));
}

void TimeEvents::blockingAddProps(QVariantMap props)
{
    Locker lock(mMutex);
    mProps.reset(new QVariantMap(props));
    Q_EMIT propsUpdateNotify();
}

void TimeEvents::blockingClear()
{
    Locker lock(mMutex);
    mTimeEvents.clear();
    Q_EMIT cleared();
}
