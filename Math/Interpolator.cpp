#include <QStringList>
#include "interpolator.h"

Interpolator::Interpolator()
{

}

Interpolator::~Interpolator()
{

}

Interpolator::Pointer Interpolator::create(const QString &name)
{
    if(name == "Linear") return Pointer(new LinInterp);
    else return Pointer();
}

const QStringList &Interpolator::names()
{
    static const QStringList s_names{ "Linear" };
    return s_names;
}

Interpolator::Vector LinInterp::interpolate
(
    const Interpolator::Vector &x,
    const Interpolator::Vector &y,
    const Interpolator::Vector &xNew,
    bool isSorted
)
{
    assert(x.size() == y.size() && x.size() >= 2);
    if(isSorted)
    {
        return interpolateSorted(x,y,xNew);
    }
    else
    {
        Vector tx(x), ty(y);
        pairSort(tx, ty);
        return interpolateSorted(tx, ty, xNew);
    }
}

Interpolator::Vector LinInterp::interpolate
(
    const Interpolator::Vector &y,
    const Interpolator::Vector &xNew
)
{
    assert(y.size() >= 2);
    const size_t N = y.size();
    Vector res(xNew.size());
    for(size_t i = 0; i < xNew.size(); ++i)
    {
        if(xNew[i] < 0) res[i] = (y[1] - y[0]) * xNew[i];
        else if(xNew[i] >= N - 1) res[i] = (y[N - 1] - y[N - 2]) * (xNew[i] - N + 2.);
        else
        {
            const size_t n = static_cast<size_t>(xNew[i]);
            res[i] = (y[n+1] - y[n]) * (xNew[i] - n);
        }
    }
    return res;
}

Interpolator::Vector LinInterp::interpolateSorted
(
    const Interpolator::Vector &x,
    const Interpolator::Vector &y,
    const Interpolator::Vector &xNew
)
{
    Vector res(xNew.size());
    for(size_t i = 0; i < xNew.size(); ++i)
    {
        res[i] = interpolateSorted(x,y,xNew[i]);
    }
    return res;
}

double LinInterp::interpolateSorted
(
    const Interpolator::Vector &x,
    const Interpolator::Vector &y,
    double xNew
)
{
    if(xNew < x.front() || xNew >= x.back()) return 0.0;
    Vector::const_iterator
            itx = std::upper_bound(x.begin(), x.end(), xNew),
            ity = y.begin();
    std::advance(ity, std::distance(x.begin(), itx));
    const double x0 = *prev(itx);
    const double x1 = *itx;
    const double y0 = *prev(ity);
    const double y1 = *ity;

    return (y1 - y0) / (x1 - x0) * (xNew - x0) + y0;
}
