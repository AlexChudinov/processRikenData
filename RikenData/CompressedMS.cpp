#include "CompressedMS.h"
#include "Math/smoothing_splines.h"
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

CompressedMS::CompressedMS(Map &&data, Interpolator::InterpType type)
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

double CompressedMS::match(const CompressedMS &msRef) const
{
    const Map& tabRef = msRef.interp()->table();
    double res = 0;

    for(const auto& e : tabRef)
        res += e.second * interp()->interpolate(e.first);

    return res;
}

double CompressedMS::bestMatch(const CompressedMS &msRef, int nMaxTime) const
{
    CompressedMS temp(*this);
    double maxMatch = 0.0;
    double smax= 0.0 ;
    for(int n = -100; n <= 100; ++n)
    {
        double s = double(n) / double(nMaxTime);
        temp.squeezeXScale(s);
        double curMatch = temp.match(msRef);
        if(maxMatch < curMatch)
        {
            maxMatch = curMatch;
            smax = s;
        }
    }

    return smax;
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
    uint32_t tMin = static_cast<uint32_t>(interp()->minX());
    uint32_t tMax = static_cast<uint32_t>(interp()->maxX()) + 1;
    VectorInt vVals(tMax - tMin + 1);
    for(size_t i = 0; i < vVals.size(); ++i)
        vVals[i] =
            std::round(interp()->interpolate(tMin + i));
    *this = CompressedMS(vVals, tMin, interp()->type());
}

void CompressedMS::logSplineSmoothing(double p)
{
    uint32_t tMin = static_cast<uint32_t>(interp()->minX());
    uint32_t tMax = static_cast<uint32_t>(interp()->maxX()) + 1;
    std::vector<double> w(tMax - tMin + 1);
    std::vector<double> y(tMax - tMin + 1, 1.0);
    for(size_t i = 0; i < y.size(); ++i)
    {
        double yVal = interp()->interpolate(i+tMin);
        w[i] = yVal <= 1.0 ? 1.0 : yVal;
        y[i] += yVal;
    }
    std::vector<double> yy(tMax - tMin + 1);
    math::log_third_order_smoothing_spline_eq
    (
        y.size(),
        y.data(),
        w.data(),
        p,
        yy.data()
    );
    w.clear();
    y.clear();

    VectorInt vVals(tMax - tMin + 1, 1);
    for(size_t i = 0; i < vVals.size(); ++i)
    {
        vVals[i] = std::round(yy[i] - 1);
    }
    yy.clear();
    *this = CompressedMS(vVals, tMin, interp()->type());
}

void CompressedMS::logSplineParamLessSmoothing()
{
    CompressedMS temp(*this);
    double param = 1.;
    uint64_t TIC = totalIonCount();
    temp.logSplineSmoothing(param);
    uint64_t s = sumSqDev(temp);
    double a, b;
    if(s > TIC)
    {
        while (s > TIC)
        {
            temp = *this;
            temp.logSplineSmoothing(param /= 10);
            s = sumSqDev(temp);
        }
        a = param; b = param * 10.;
    }
    else
    {
        while(s < TIC)
        {
            temp = *this;
            temp.logSplineSmoothing(param *= 10.);
            s = sumSqDev(temp);
        }
        a = param/10.; b = param;
    }

    if(s != TIC) while(true)
    {
        param = (a + b)/2;
        temp = *this;
        temp.logSplineSmoothing(param);
        s = sumSqDev(temp);
        if(s < TIC) a = param;
        else if (s > TIC) b = param;
        else break;
    }

    logSplineSmoothing(param);
}

CompressedMS::uint64_t CompressedMS::sumSqDev(const CompressedMS &ms) const
{
    CompressedMS::uint64_t res = 0;
    for(const auto& e : ms.interp()->table()) res += e.second * e.second;
    for(const auto& e : interp()->table())
    {
        CompressedMS::Map::const_iterator it = ms.interp()->table().find(e.first);
        if (it != ms.interp()->table().end())
        {
            res -= it->second * it->second;
            res += (it->second - e.second) * (it->second - e.second);
        }
        else
            res += e.second * e.second;
    }
    return res;
}

CompressedMS::uint64_t CompressedMS::totalIonCount() const
{
    uint64_t res = 0;
    for(const auto& e : interp()->table()) res += e.second;
    return res;
}
