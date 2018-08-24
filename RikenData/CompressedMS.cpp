#include "CompressedMS.h"
#include <cmath>

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

    do
    {
        temp.squeezeXScale(double(n-1)/double(nMaxTime));
        pl = temp.match(msRef);
        temp.squeezeXScale(double(n)/double(nMaxTime));
        p0 = temp.match(msRef);
        temp.squeezeXScale(double(n+1)/double(nMaxTime));
        pr = temp.match(msRef);

        if(std::min(pl,std::min(p0, pr)) == p0 || pl != pr)
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

