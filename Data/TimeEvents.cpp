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
        flushTimeSlice();
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

void TimeEvents::flushTimeSlice()
{
    mTimeEvents.append(mTimeEventsSlice);
    Q_EMIT eventsUpdateNotify(static_cast<size_t>(mTimeEvents.size()));
    Q_EMIT sliceAccumulated(mTimeEventsSlice);
    mTimeEventsSlice.clear();
}



void TimeEvents::blockingFlushTimeSlice()
{
    Locker lock(mMutex);
    flushTimeSlice();
}

void TimeEvents::recalculateTimeSlices(size_t startsPerHist)
{
    Q_EMIT beforeRecalculation();
    Locker lock(mMutex);
    if(!mTimeEvents.empty() && startsPerHist != 0 && mStartsPerHist != startsPerHist)
    {
        MyInit::instance()->massSpec()->blockingClear();
        mStartsPerHist = startsPerHist;
        mStartsCount = 0;
        mTimeEventsSlice.clear();

        for(TimeEvent evt : mTimeEvents)
        {
            if(!evt && mStartsCount++ == mStartsPerHist)
            {
                mStartsCount = 1;
                Q_EMIT sliceAccumulated(mTimeEventsSlice);
                mTimeEventsSlice.clear();
                mTimeEventsSlice.push_back(evt);
            }
            else
            {
                mTimeEventsSlice.push_back(evt);
            }
        }
        if(mTimeEventsSlice.size() != 1) Q_EMIT sliceAccumulated(mTimeEventsSlice);
    }
    Q_EMIT recalculated();
}

size_t TimeEvents::startsPerHist()
{
    Locker lock(mMutex);
    return mStartsPerHist;
}
