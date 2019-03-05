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
    qRegisterMetaType<MapUintUint>("MapUintUint");
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
                it = currHist->insert({evt, 1}).first;
                if(it == currHist->begin() || std::prev(it)->first != evt - 1)
                {
                    currHist->insert(it, {evt - 1, 0});
                }
                if(std::next(it) == currHist->end() || std::next(it)->first != evt + 1)
                {
                    currHist->insert(std::next(it), {evt + 1, 0});
                }
            }
            else
            {
                if(it->second == 0)
                {
                    if(it == currHist->begin() || std::prev(it)->first != evt - 1)
                    {
                        currHist->insert(it, {evt - 1, 0});
                    }
                    else if(std::next(it) == currHist->end() || std::next(it)->first != evt + 1)
                    {
                        currHist->insert(it, {evt + 1, 0});
                    }
                }
                it->second ++;
            }
        }
    }
    if(!currHist->empty())
    {
        mMinTimeBin = qMin(mMinTimeBin, currHist->begin()->first);
        mMaxTimeBin = qMax(mMaxTimeBin, currHist->rbegin()->first);
        Q_EMIT timeLimitsNotify(mMinTimeBin, mMaxTimeBin);
        Q_EMIT massSpecsNumNotify(mData.size());
    }
    else mData.pop_back(); //No events occur for some reason
}

void MassSpec::addMassSpec(const MapUintUint &ms)
{
    if(ms.empty()) return;
    mData.push_back(ms);
    mMinTimeBin = qMin(mMinTimeBin, ms.begin()->first);
    mMaxTimeBin = qMax(mMaxTimeBin, ms.rbegin()->first);
    Q_EMIT timeLimitsNotify(mMinTimeBin, mMaxTimeBin);
    Q_EMIT massSpecsNumNotify(mData.size());
}

void MassSpec::blockingAddMassSpec(const MapUintUint &ms)
{
    Locker lock(mMutex);
    addMassSpec(ms);
}

MapUintUint MassSpec::getMassSpec(size_t First, size_t Last) const
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

MapUintUint MassSpec::blockingGetMassSpec(size_t First, size_t Last)
{
    Locker lock(mMutex);
    return getMassSpec(First, Last);
}

const MapUintUint &MassSpec::getMassSpec(size_t num) const
{
    return mData[num];
}

MapUintUint MassSpec::blockingGetMassSpec(size_t num)
{
    Locker lock(mMutex);
    return getMassSpec(num);
}

double MassSpec::getMassSpecTotalCurrent(size_t idx) const
{
    Q_ASSERT(idx < mData.size());

    double res = 0.0;

    for(MapUintUint::const_reference d : mData[idx])
    {
        res += d.second;
    }

    return res;
}

double MassSpec::blockingGetMassSpecTotalCurrent(size_t idx)
{
    Locker lock(mMutex);
    return getMassSpecTotalCurrent(idx);
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

MapUintUint MassSpec::lastMS() const
{
    Q_ASSERT(!mData.empty());
    return *mData.rbegin();
}

MapUintUint MassSpec::blockingLastMS()
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

std::pair<Uint, Uint> MassSpec::minMaxTime() const
{
    return std::make_pair(mMinTimeBin, mMaxTimeBin);
}

std::pair<Uint, Uint> MassSpec::blockingMinMaxTime()
{
    Locker lock(mMutex);
    return minMaxTime();
}
const QVariantMap &MassSpec::massSpecRelatedData() const
{
    return mMassSpecRelatedData;
}

QVariantMap &MassSpec::massSpecRelatedData()
{
    return mMassSpecRelatedData;
}

MassSpec::Locker MassSpec::lockInstance()
{
    return Locker(mMutex);
}

