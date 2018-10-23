#include "Base/BaseObject.h"
#include "MassSpec.h"
#include <QMessageBox>

MassSpec::MassSpec(QObject *parent)
    :
      QObject(parent),
      mMinTimeBin(std::numeric_limits<Uint>::max()),
      mMaxTimeBin(std::numeric_limits<Uint>::min())
{
    setObjectName("MassSpec");
    qRegisterMetaType<Uint>("Uint");
}

void MassSpec::clear()
{
    mMinTimeBin = std::numeric_limits<Uint>::max();
    mMaxTimeBin = std::numeric_limits<Uint>::min();
    mData.clear();
    Q_EMIT cleared();
    Q_EMIT massSpecsNumNotify(0);
    Q_EMIT timeLimitsNotify(mMinTimeBin, mMaxTimeBin);
}

void MassSpec::blockingClear()
{
    Locker lock(mMutex);
    clear();
}

void MassSpec::blockingNewHist(TimeEventsContainer evts)
{
    Locker lock(mMutex);
    mData.push_back(MapUintUint());
    HistCollection::reverse_iterator currHist = mData.rbegin();
    for(auto evt : evts)
    {
        if(evt)
        {
            MapUintUint::iterator it = currHist->find(evt);
            if(it == currHist->end())
            {
                currHist->operator[](evt) = 1;
            }
            else
            {
                it->second ++;
            }
        }
    }
    mMinTimeBin = qMin(mMinTimeBin, currHist->begin()->first);
    mMaxTimeBin = qMax(mMaxTimeBin, currHist->rbegin()->first);
    Q_EMIT timeLimitsNotify(mMinTimeBin, mMaxTimeBin);
    Q_EMIT massSpecsNumNotify(mData.size());
}

MassSpec::MapUintUint MassSpec::getMassSpec(size_t First, size_t Last) const
{
    Q_ASSERT(First < Last && Last <= mData.size());
    MapUintUint res;
    for(; First < Last; ++First)
    {
        const MapUintUint& curHist = mData[First];
        for(MapUintUint::const_reference r : curHist)
        {
            MapUintUint::iterator it = res.find(r.first);
            if (it == res.end())
            {
                res[r.first] = r.second;
            }
            else
            {
                it->second += r.second;
            }
        }
    }
    return res;
}

MassSpec::MapUintUint MassSpec::blockingGetMassSpec(size_t First, size_t Last)
{
    Locker lock(mMutex);
    return getMassSpec(First, Last);
}

MassSpec::VectorUint MassSpec::getIonCurrent(Uint First, Uint Last) const
{
    VectorUint res(mData.size());
    size_t i = 0;
    for(const MapUintUint& currHist : mData)
    {
        MapUintUint::const_iterator
                it1 = currHist.lower_bound(First),
                it2 = currHist.upper_bound(Last);
        size_t TIC = 0;
        for(;it1 != it2; ++it1)
        {
            TIC += it1->second;
        }
        res[i++] = TIC;
    }
    return res;
}

MassSpec::VectorUint MassSpec::blockingGetIonCurrent(Uint First, Uint Last)
{
    Locker lock(mMutex);
    return getIonCurrent(First, Last);
}

MassSpec::MapUintUint MassSpec::lastMS() const
{
    Q_ASSERT(!mData.empty());
    return *mData.rbegin();
}

MassSpec::MapUintUint MassSpec::blockingLastMS()
{
    Locker lock(mMutex);
    return lastMS();
}

double MassSpec::lastTic(Uint First, Uint Last) const
{
    Q_ASSERT(!mData.empty());
    return std::accumulate(mData.crbegin()->lower_bound(First),
                           mData.crbegin()->upper_bound(Last), 0.0,
                           [](double a, MapUintUint::const_reference r)->double
    {
        return a + r.second;
    });
}

double MassSpec::blockingLastTic(Uint First, Uint Last)
{
    Locker lock(mMutex);
    return lastTic(First, Last);
}

size_t MassSpec::size() const
{
    return mData.size();
}

size_t MassSpec::blockingSize()
{
    Locker lock(mMutex);
    return size();
}

std::pair<MassSpec::Uint, MassSpec::Uint> MassSpec::minMaxTime() const
{
    return std::make_pair(mMinTimeBin, mMaxTimeBin);
}

std::pair<MassSpec::Uint, MassSpec::Uint> MassSpec::blockingMinMaxTime()
{
    Locker lock(mMutex);
    return minMaxTime();
}
