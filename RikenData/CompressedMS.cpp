#include "CompressedMS.h"
#include <cmath>

CompressedMS::CompressedMS(const CompressedMS::VectorInt &vVals,
                           size_t tMin, Interpolator::InterpType type)
{
    size_t x0 = 0, x1 = 1;
    while(x1 < vVals.size() && vVals[x1] == 0) x0 = x1++;
    CompressedMS::Map tab;
    bool bPeakBack = false;
    while(x1 < vVals.size())
    {
        //If it is equal to zero, then check previous value
        if(vVals[x1] == 0)
        {
            //...ok, it is not zero, then write it
            if(vVals[x0] != 0)
            {
                bPeakBack = true;
                tab[x1 + tMin] = vVals[x1];
            }
            else bPeakBack = false;
            //...otherwise skip it
        }
        else //write just not zero values
        {
            //If previous value was equal to zero and it was not directly behind previous peak
            //...then write it too
            if(vVals[x0] == 0 && !bPeakBack) tab[x0 + tMin] = 0;
            tab[x1 + tMin] = vVals[x1];
        }
        x0 = x1++;
    }
    m_pInterpolator.reset(Interpolator::create(type, std::move(tab)).release());
}

CompressedMS::CompressedMS(const Map& data, Interpolator::InterpType type)
    :
      m_pInterpolator(Interpolator::create(type, data))
{
}

CompressedMS::CompressedMS(Map &&data, IntegerInterpolator::InterpType type)
    :
      m_pInterpolator(Interpolator::create(type, data))
{
}

CompressedMS::CompressedMS(const CompressedMS &ms)
    :
      m_pInterpolator(Interpolator::create(ms.interp()->type(), ms.interp()->table()))
{
}

CompressedMS::CompressedMS(CompressedMS&& ms)
    :
      m_pInterpolator(Interpolator::create(ms.interp()->type(), std::move(ms.interp()->table())))
{
}

CompressedMS &CompressedMS::operator=(const CompressedMS &ms)
{
    m_pInterpolator.reset(Interpolator::create(ms.interp()->type(), ms.interp()->table()).release());
    return *this;
}

CompressedMS &CompressedMS::operator=(CompressedMS&& ms)
{
    m_pInterpolator.reset
    (
        Interpolator::create(ms.interp()->type(), std::move(ms.interp()->table())).release()
    );
    return *this;
}


CompressedMS::~CompressedMS()
{
}

void CompressedMS::squeezeXScale(double s)
{
    if(s>-1.0 && s<1.0) m_pInterpolator->xFactor(1. + s);
}

CompressedMS::uint64_t CompressedMS::match(const CompressedMS &msRef) const
{
    const Map& tabRef = msRef.interp()->table();
    uint64_t res = 0;

    for(const auto& e : tabRef)
        res += e.second * std::round(interp()->interpolate(static_cast<double>(e.first)));

    return res;
}

double CompressedMS::bestMatch(const CompressedMS &msRef, int nMaxTime, bool *ok) const
{
    CompressedMS temp(*this);
    //These variables keep match values calculated for different time scale shifts
    uint64_t pl, p0, pr;
    int n = 0; //Current shift value is s = n/nMaxTime = 0

    const size_t maxIterNum = 10000; //restricts the number of iterations
    size_t curIterNum = 0;
    do
    {
        temp.squeezeXScale(double(n-1)/double(nMaxTime));
        pl = temp.match(msRef);
        temp.squeezeXScale(double(n)/double(nMaxTime));
        p0 = temp.match(msRef);
        temp.squeezeXScale(double(n+1)/double(nMaxTime));
        pr = temp.match(msRef);

        if(std::min(pl,std::min(p0, pr)) == p0 || pl != pr || ++curIterNum == maxIterNum)
        {
            if(ok) *ok = false;
            return double(n)/double(nMaxTime);
        }

        if(pr > pl) n++;
        else n--;
    }while(std::max(pl,std::max(p0, pr)) != p0);
    if(ok) *ok = true;
    return double(n)/double(nMaxTime);
}

void CompressedMS::addToAcc(CompressedMS &msAcc) const
{
    const Map& tab = interp()->table();
    Map& tabAcc = msAcc.interp()->table();
    for(const auto& e : tab)
    {
        Map::iterator it = tabAcc.find(e.first);
        if(it != tabAcc.end()) it->second += e.second;
        else tabAcc[e.first] = e.second;
    }
}

void CompressedMS::rescale()
{
    uint64_t tMin = static_cast<uint32_t>(interp()->minX());
    uint64_t tMax = static_cast<uint32_t>(interp()->maxY()) + 1;
    VectorInt vVals(tMax - tMin + 1);
    for(size_t i = 0; i < vVals.size(); ++i)
        vVals[i] = interp()->interpolate(static_cast<double>(tMin + i));
    *this = CompressedMS(vVals, tMin, interp()->type());
}

