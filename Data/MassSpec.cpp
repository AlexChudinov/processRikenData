#include "Base/BaseObject.h"
#include "MassSpec.h"
#include <QMessageBox>

MassSpec::MassSpec(QObject *parent)
    :
      QObject(parent)
{
    setObjectName("MassSpec");
}

void MassSpec::clear()
{
    mData.clear();
    Q_EMIT cleared();
    Q_EMIT massSpecsNumNotify(0);
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
    Q_EMIT massSpecsNumNotify(mData.size());
}

MassSpec::MapUintUint MassSpec::getMassSpec(size_t First, size_t Last) const
{
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

MassSpec::MapUintUint MassSpec::getIonCurrent(size_t First, size_t Last) const
{
    MapUintUint res;
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

MassSpec::MapUintUint MassSpec::blockingGetIonCurrent(size_t First, size_t Last)
{
    Locker lock(mMutex);
    return getIonCurrent(First, Last);
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

unsigned long long MassSpec::maxTime() const
{
    unsigned long long res = 0;
    for(const MapUintUint& currHist : mData)
    {
        res = qMax(res, std::max_element
                   (
                       currHist.begin(),
                       currHist.end(),
                       [](MapUintUint::const_reference a, MapUintUint::const_reference b)->bool
        { return a.second < b.second; }
                   )->second);
    }
    return res;
}

unsigned long long MassSpec::blockingMaxTime()
{
    Locker lock(mMutex);
    return maxTime();
}
