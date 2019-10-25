#include "Data/MassSpec.h"
#include "MassSpecSummator.h"
#include "Base/ThreadPool.h"
#include <QtConcurrent>

MassSpecSummator::MassSpecSummator()
{

}

MassSpecSummator::~MassSpecSummator()
{

}

MapUintUint &MassSpecSummator::add(MapUintUint &lhMs, const MapUintUint &rhMs) const
{
    for(MapUintUint::const_reference d : rhMs)
    {
        MapUintUint::iterator it = lhMs.find(d.first);
        if(it != lhMs.end()) it->second += d.second;
        else lhMs.insert(d);
    }
    return lhMs;
}

MapIntInt &MassSpecSummator::add(MapIntInt &lhMs, const MapIntInt &rhMs) const
{
    for(MapIntInt::const_reference d : rhMs)
    {
        MapIntInt::iterator it = lhMs.find(d.first);
        if(it != lhMs.end()) it->second += d.second;
        else lhMs.insert(d);
    }
    return lhMs;
}

MSSum::MSSum()
{
}

MSSum::~MSSum()
{
}

MapIntInt DirectSum::accum
(
    MassSpectrumsCollection *coll,
    size_t _First,
    size_t _Last
)
{
    QMutexLocker lock(&coll->mMut);
    const int n = coll->nMaxBin - coll->nMinBin + 1;
    std::vector<int> acc(static_cast<size_t>(n));
    for(; _First != _Last && _First < _Last; ++_First)
    {
        MassSpecImpl::MapShrdPtr msDataPtr = coll->mCollection[_First]->data();
        for(MassSpecImpl::Map::const_reference d : *msDataPtr)
        {
            acc[d.first - coll->nMinBin] += d.second;
        }
    }
    MapIntInt res;
    MapIntInt::const_iterator it = res.end();
    for(size_t i = 0; i < acc.size(); ++i)
    {
        if(acc[i] != 0.0)
        {
            it = res.insert(it, {i + coll->nMinBin, acc[i]});
            ++it;
        }
    }
    it = res.insert(res.begin(), {res.begin()->first - 1, 0});
    while(++it != res.end())
    {
        if(std::prev(it)->first + 1 != it->first)
        {
            res.insert(it, {it->first - 1, 0});
        }
        if(std::next(it) == res.end() || std::next(it)->first - 1 != it->first)
        {
            res.insert(std::next(it), {it->first + 1, 0});
            ++it;
        }
    }
    return res;
}
