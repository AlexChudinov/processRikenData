#include "MassSpecSummator.h"

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
