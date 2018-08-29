#include "CompressedMS.h"
#include "Math/smoothing_splines.h"
#include <QtConcurrent>
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

    for(Map::const_reference e : tabRef)
        res += e.second * interp()->interpolate(e.first);

    return res;
}

double CompressedMS::bestMatch(const CompressedMS &msRef, size_t nMaxTime, size_t nTimeInterval) const
{
    VectorDouble vMatchVals(2*nTimeInterval + 1);
    VectorInt vIdx(vMatchVals.size());
    std::iota(vIdx.begin(), vIdx.end(), 0);
    VectorDouble sVals(vMatchVals.size());

    QtConcurrent::map<VectorInt>(vIdx, [&](size_t i)->void
    {
        CompressedMS temp(*this);
        sVals[i] = (double(i) - double(nTimeInterval)) / double(nMaxTime);
        temp.squeezeXScale(sVals[i]);
        vMatchVals[i] = temp.match(msRef);
    }).waitForFinished();

    return sVals
    [
        static_cast<size_t>(std::distance
        (
            std::max_element(vMatchVals.begin(), vMatchVals.end()), vMatchVals.begin()
        ))
    ];
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
    VectorInt vRange(vVals.size());
    std::iota(vRange.begin(), vRange.end(), 0);
    QtConcurrent::map<VectorInt>(vRange, [&](size_t i)->void
    {
        vVals[i] = static_cast<size_t>(std::round(interp()->interpolate(i+tMin)));
    }).waitForFinished();

    *this = CompressedMS(vVals, tMin, interp()->type());
}

void CompressedMS::logSplineSmoothing(double p)
{
    uint32_t tMin = static_cast<uint32_t>(interp()->minX());
    uint32_t tMax = static_cast<uint32_t>(interp()->maxX());
    VectorDouble w(tMax - tMin + 1, 1.0);
    VectorDouble y(tMax - tMin + 1, 1.0);

    for(Map::const_reference e : interp()->table())
    {
        w[static_cast<size_t>(e.first) - tMin] = e.second <= 1 ? 1.0 : e.second;
        y[static_cast<size_t>(e.first) - tMin] += e.second;
    }

    std::vector<double> yy(tMax - tMin + 1);
    math::log_third_order_smoothing_spline_eq
    (
        static_cast<int>(y.size()),
        y.data(),
        w.data(),
        p,
        yy.data()
    );
    w.clear();
    y.clear();

    VectorInt vVals(tMax - tMin + 1, 1);
    VectorInt Idx(vVals.size()); //prepare vector of array indices
    std::iota(Idx.begin(), Idx.end(), 0);
    QtConcurrent::map<VectorInt>(Idx, [&](size_t i)->void
    {
        vVals[i] = static_cast<size_t>(std::round(yy[i] - 1));
    }).waitForFinished();

    yy.clear();
    *this = CompressedMS(vVals, tMin, interp()->type());
}

void CompressedMS::logSplineParamLessSmoothing()
{
    uint32_t tMin = static_cast<uint32_t>(interp()->minX());
    uint32_t tMax = static_cast<uint32_t>(interp()->maxX());

    uint64_t TIC = totalIonCount();
    double p = 1.; //Init. smooth param. val

    VectorDouble w(tMax - tMin + 1, 1.0);
    VectorDouble y(w.size(), 1.0);

    for(Map::const_reference e : interp()->table())
    {
        w[static_cast<size_t>(e.first) - tMin] = e.second <= 1 ? 1.0 : e.second;
        y[static_cast<size_t>(e.first) - tMin] += e.second;
    }

    VectorDouble yy(w.size()); //prepare vector to hold smoothed vals

    math::log_third_order_smoothing_spline_eq
    (
        static_cast<int>(y.size()),
        y.data(),
        w.data(),
        p,
        yy.data()
    );

    auto sqDiffFun = [](double a, double b)->double{ return (a-b)*(a-b); };

    double s = std::inner_product
    (
        y.begin(),
        y.end(),
        yy.begin(),
        0.0,
        std::plus<double>(),
        sqDiffFun
    );

    double a, b;
    if(s > TIC)
    {
        while (s > TIC)
        {
            math::log_third_order_smoothing_spline_eq
            (
                static_cast<int>(y.size()),
                y.data(),
                w.data(),
                p /= 10.,
                yy.data()
            );

            s = std::inner_product
            (
                y.begin(),
                y.end(),
                yy.begin(),
                0.0,
                std::plus<double>(),
                sqDiffFun
            );
        }
        a = p; b = p * 10.;
    }
    else
    {
        while(s < TIC)
        {
            math::log_third_order_smoothing_spline_eq
            (
                static_cast<int>(y.size()),
                y.data(),
                w.data(),
                p *= 10.,
                yy.data()
            );

            s = std::inner_product
            (
                y.begin(),
                y.end(),
                yy.begin(),
                0.0,
                std::plus<double>(),
                sqDiffFun
            );
        }
        a = p / 10.; b = p;
    }

    if(static_cast<size_t>(std::round(s)) != TIC)
    while(std::abs(a - b) > 1.0)
    {
        math::log_third_order_smoothing_spline_eq
        (
            static_cast<int>(y.size()),
            y.data(),
            w.data(),
            p = .5 * (a + b),
            yy.data()
        );

        s = std::inner_product
        (
            y.begin(),
            y.end(),
            yy.begin(),
            0.0,
            std::plus<double>(),
            sqDiffFun
        );

        if(static_cast<size_t>(std::round(s)) < TIC) a = p;
        else if (static_cast<size_t>(std::round(s)) > TIC) b = p;
        else break;
    }

    logSplineSmoothing(p);
}

Peak::PeakCollection CompressedMS::getPeaks(double p) const
{
    CompressedMS tmp(*this);
    tmp.rescale();
    tmp.logSplineSmoothing(p);
    VectorDouble y = tmp.transformToVector();
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

CompressedMS::VectorDouble CompressedMS::transformToVector() const
{
    size_t
            tMin = static_cast<size_t>(interp()->minX()),
            tMax = static_cast<size_t>(interp()->maxX());
    VectorDouble res(tMax - tMin + 1);
    for(const auto& e : interp()->table())
        res[e.first - tMin] = e.second;
    return res;
}

double Peak::right() const
{
    return m_right;
}

void Peak::setRight(double right)
{
    m_right = right;
}

double Peak::center() const
{
    return m_center;
}

void Peak::setCenter(double center)
{
    m_center = center;
}

double Peak::height() const
{
    return m_height;
}

void Peak::setHeight(double height)
{
    m_height = height;
}

double Peak::left() const
{
    return m_left;
}

void Peak::setLeft(double left)
{
    m_left = left;
}
