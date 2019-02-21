#ifndef MASSSPECSUMMATOR_H
#define MASSSPECSUMMATOR_H

#include "Data/MassSpec.h"

class MassSpecSummator
{
public:
    MassSpecSummator();
    virtual ~MassSpecSummator();

    virtual MapUintUint& add(MapUintUint& lhMs, const MapUintUint& rhMs) const;
};

#endif // MASSSPECSUMMATOR_H
