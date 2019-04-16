#ifndef MASSSPECSUMMATOR_H
#define MASSSPECSUMMATOR_H

#include <map>
#include <Data/MassSpecImpl.h>

using Uint = unsigned long long;
using MapUintUint = std::map<Uint, Uint>;
using MapIntInt = std::map<int, int>;
class MassSpectrumsCollection;

class MassSpecSummator
{
public:
    MassSpecSummator();
    virtual ~MassSpecSummator();

    virtual MapUintUint& add(MapUintUint& lhMs, const MapUintUint& rhMs) const;

    virtual MapIntInt& add(MapIntInt& lhMs, const MapIntInt& rhMs) const;
};

class MSSum
{
public:
    MSSum();
    virtual ~MSSum();

    /**
     * @brief accum accumulates mass spectrums
     * @return
     */
    virtual MapIntInt accum
    (
        MassSpectrumsCollection * coll,
        size_t _First,
        size_t _Last
    ) = 0;
};

class DirectSum : public MSSum
{
public:
    MapIntInt accum
    (
        MassSpectrumsCollection * coll,
        size_t _First,
        size_t _Last
    );
};

#endif // MASSSPECSUMMATOR_H
