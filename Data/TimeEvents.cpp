#include "Data/MassSpec.h"
#include "Base/BaseObject.h"
#include "TimeEvents.h"

TimeEvents::TimeEvents(QObject *parent)
    :
      QObject(parent),
      mStartsPerHist(1000),
      mStartsCount(0)
{
    qRegisterMetaType<TimeEventsContainer>("TimeEventsContainer");
    setObjectName("TimeEvents");
}

TimeEvents::~TimeEvents()
{

}

void TimeEvents::blockingAddEvent(TimeEvent evt)
{
    Locker lock(mMutex);

    if(!evt && mStartsCount++ == mStartsPerHist)
    {
        mStartsCount = 1;
        mTimeEvents.append(mTimeEventsSlice);
        Q_EMIT eventsUpdateNotify(static_cast<size_t>(mTimeEvents.size()));
        Q_EMIT sliceAccumulated(mTimeEventsSlice);
        mTimeEventsSlice.clear();
        mTimeEventsSlice.push_back(evt);
    }
    else
    {
        mTimeEventsSlice.push_back(evt);
    }
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
    mStartsCount = 0;
    mTimeEvents.clear();
    mTimeEventsSlice.clear();
    Q_EMIT cleared();
}

TimeEventsContainer TimeEvents::timeEventsSlice() const
{
    return mTimeEventsSlice;
}
