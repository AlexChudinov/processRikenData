#include "Base/BaseObject.h"
#include "MassSpec.h"
#include "Data/TimeEvents.h"

MassSpec::MassSpec(QObject *parent)
    :
      QObject(parent),
      mStartsCount(0),
      mEventsCount(0),
      mStartsPerHist(1000),
      mData(1) //at least one mass spectrum by default
{
    setObjectName("MassSpec");
    Q_ASSERT(mEvents = MyInit::instance().findChild<TimeEvents*>("TimeEvents"));
}

void MassSpec::clear()
{
    mData.assign(1, MapUintUint());
    mStartsCount = 0;
    mEventsCount = 0;
    Q_EMIT cleared();
}

void MassSpec::blockingClear()
{
    Locker lock(mMutex);
    clear();
}

void MassSpec::accumulateEvents(size_t nTimeEventsCount)
{
    for(; mEventsCount < nTimeEventsCount; ++mEventsCount)
    {
        TimeEvents::TimeEvent evt(mEvents->mTimeEvents[static_cast<int>(mEventsCount)]);
        if(!evt)
        {
            HistCollection::reverse_iterator currHist = mData.rbegin();
            MapUintUint::iterator it = currHist->find(evt);
            if(it != currHist->end())
            {
                it->second ++;
            }
            else
            {
                currHist->operator[](evt) ++;
            }
        }
        else
        {
            mStartsCount++;
            if(mStartsCount / mStartsPerHist + 1 != mData.size())
                mData.push_back(MapUintUint());
        }
    }
    Q_EMIT changed();
}

void MassSpec::blockingAccumulateEvents(size_t nTimeEventsCount)
{
    Locker lock(mMutex);
    accumulateEvents(nTimeEventsCount);
}

void MassSpec::setStartsPerHist(size_t startsPerHist)
{
    clear();
    mStartsPerHist = startsPerHist;
    accumulateEvents(static_cast<size_t>(mEvents->mTimeEvents.size()));
}

void MassSpec::blockingSetStartsPerHist(size_t startsPerHist)
{
    Locker lock(mMutex);
    setStartsPerHist(startsPerHist);
}
