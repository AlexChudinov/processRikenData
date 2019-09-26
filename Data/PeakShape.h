#ifndef PEAKSHAPE_H
#define PEAKSHAPE_H

#include <vector>
#include <memory>
#include <QTextStream>
#include "../Math/interpolator.h"

/**
 * @brief The PeakShape class fits any peak with 3 parameters
 */
class PeakShape
{
public:
    using Pointer = std::unique_ptr<PeakShape>;
    using Vector = Interpolator::Vector;

    PeakShape();
    virtual ~PeakShape();

    enum Type
    {
        InterpolatorFunType
    };

    static const QStringList& names();
    static Pointer create(const QString& name);
    static Pointer create(Type type);

    virtual Type type() const = 0;

    virtual bool valid() const = 0;

    virtual double peakPosition() const = 0;
    virtual void setPeakPosition(double) = 0;

    virtual double peakWidth() const = 0;
    virtual void setPeakWidth(double) = 0;

    virtual double peakAmp() const = 0;
    virtual void setPeakAmp(double) = 0;

    virtual Vector values(const Vector&) const = 0;

    virtual void import(QTextStream& out) const = 0;
protected:

};


/**
 * @brief The InterpolatorFun class using vector of x, y values to interpolate
 */
class InterpolatorFun : public PeakShape
{
    std::vector<double> m_vXVals, m_vYVals;
    double mPeakPosition, mPeakWidth, mPeakAmp;
    Interpolator::Pointer mInterp;
public:
    InterpolatorFun(const QString& strInterp = "Linear");

    InterpolatorFun(const Vector& xVals, const Vector& yVals, const QString& strInterp = "Linear");

    InterpolatorFun(const InterpolatorFun& interp);

    void setXYValues(const Vector& xVals, const Vector& yVals, bool isSorted = true);

    Type type() const;

    bool valid() const;

    double peakPosition() const;
    void setPeakPosition(double pos);

    double peakWidth() const;
    void setPeakWidth(double width);

    double peakAmp() const;
    void setPeakAmp(double amp);

    Vector values(const Vector& x) const;

    void import(QTextStream& out) const;
};

#endif // PEAKSHAPE_H
