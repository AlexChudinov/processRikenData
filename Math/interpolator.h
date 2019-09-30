#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <vector>
#include <cassert>
#include <algorithm>
#include <QString>
#include <memory>

/**
 * @brief The Interpolator class interface for interpolating procedures
 */
class Interpolator
{
public:
    using Vector = std::vector<double>;
    using Pointer = std::unique_ptr<Interpolator>;

    Interpolator();
    virtual ~Interpolator();

    /**
     * @brief create returns corresponding interpolator
     * @param name
     * @return
     */
    static Pointer create(const QString& name);

    /**
     * @brief name returns name of a current interpolator
     * @return
     */
    virtual const QString& name() const = 0;

    /**
     * @brief names names of available interpolators
     * @return
     */
    static const QStringList& names();

    /**
     * @brief interpolate interpolates data from old x-values to a new one
     * @param x x-values
     * @param y corresponent y-values
     * @param xNew new x-values
     * @param isSorted is true if x-values was already sorted otherwise they will be sorted inside function
     * @return y-values corresponing to xNew values
     */
    virtual Vector interpolate(const Vector& x, const Vector& y, const Vector& xNew, bool isSorted = true) = 0;

    /**
     * @brief interpolate same as above assuming that x-values are subsequent indexes [0, 1, ...]
     * @param y
     * @param xNew
     * @return
     */
    virtual Vector interpolate(const Vector& y, const Vector& xNew) = 0;

    virtual double interpolate(const Vector& x, const Vector& y, double x0) = 0;

    /**
     * @brief equalStepData estimates y-values corresponednt to equal step
     * from max to min x-values taken with smallest step
     * @param x
     * @param y
     * @param step returns estimated smallest step here
     * @param isSorted
     * @return
     */
    inline Vector equalStepData(const Vector& x, const Vector& y, double& step, bool isSorted = true);

    /**
     * @brief pairSort sorts pairs {x,y} and makes x array sorted and y array rearranged accordingly
     * @param x
     * @param y
     */
    static inline void pairSort(Vector& x, Vector& y);

protected:
    inline Vector equalStepDataSorted(const Vector& x, const Vector& y, double& step);
};

class LinInterp : public Interpolator
{
public:
    virtual Vector interpolate(const Vector& x, const Vector& y, const Vector& xNew, bool isSorted = true);
    virtual Vector interpolate(const Vector& y, const Vector& xNew);
    virtual double interpolate(const Vector &x, const Vector &y, double x0);

    virtual const QString& name() const
    {
        const static QString name = "Linear";
        return  name;
    }
private:
    Vector interpolateSorted(const Vector& x, const Vector& y, const Vector& xNew);
    double interpolateSorted(const Vector& x, const Vector& y, double xNew);
};

void Interpolator::pairSort(Interpolator::Vector &x, Interpolator::Vector &y)
{
    const size_t N = x.size();
    std::vector<std::pair<double, double>> pairs(x.size());
    for(size_t i = 0; i < N; ++i)
    {
        pairs[i] = {x[i], y[i]};
    }
    std::sort(pairs.begin(), pairs.end());
    for(size_t i = 0; i < N; ++i)
    {
        x[i] = pairs[i].first;
        y[i] = pairs[i].second;
    }
}

Interpolator::Vector Interpolator::equalStepData
(
    const Interpolator::Vector &x,
    const Interpolator::Vector &y,
    double& step,
    bool isSorted
)
{
    assert(x.size() == y.size() && x.size() >= 2);
    if(isSorted)
    {
        return equalStepDataSorted(x, y, step);
    }
    else
    {
        Vector tx(x), ty(y);
        pairSort(tx, ty);
        return equalStepDataSorted(tx, ty, step);
    }
}

Interpolator::Vector Interpolator::equalStepDataSorted
(
    const Interpolator::Vector &x,
    const Interpolator::Vector &y,
    double & step
)
{
    double xmin = x.front();
    double xmax = x.back();
    step = std::numeric_limits<double>::max();
    for(size_t i = 1; i < x.size(); ++i)
    {
        step = std::min(step, x[i] - x[i-1]);
    }
    Vector xNew(static_cast<size_t>((xmax - xmin) / step));
    xmin -= step;
    for(double & xx : xNew) xx = xmin += step;
    return interpolate(x, y, xNew, true);
}
#endif // INTERPOLATOR_H
