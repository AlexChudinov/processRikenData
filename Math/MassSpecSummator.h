#ifndef MASSSPECSUMMATOR_H
#define MASSSPECSUMMATOR_H

#include <map>

using Uint = unsigned long long;
using MapUintUint = std::map<Uint, Uint>;

class MassSpecSummator
{
public:
    MassSpecSummator();
    virtual ~MassSpecSummator();

    virtual MapUintUint& add(MapUintUint& lhMs, const MapUintUint& rhMs) const;
};

#endif // MASSSPECSUMMATOR_H
