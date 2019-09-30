#include "PeakShape.h"
#include <QStringList>
#include "Math/alglib/interpolation.h"
#include "Math/Solvers.h"

PeakShape::PeakShape()
{

}

PeakShape::~PeakShape()
{

}

const QStringList &PeakShape::names()
{
    static const QStringList names{ "Data-points" };
    return names;
}

PeakShape::Pointer PeakShape::create(const QString &name)
{
    if(name == names()[0]) return create(InterpolatorFunType);
    else return Pointer();
}

PeakShape::Pointer PeakShape::create(PeakShape::Type type)
{
    switch(type)
    {
    case InterpolatorFunType:
        return Pointer(new InterpolatorFun);
    }
    return Pointer();
}

InterpolatorFun::InterpolatorFun(const QString &strInterp)
    :
      mPeakPosition(0.0),
      mPeakWidth(1.0),
      mPeakAmp(1.0),
      mInterp(Interpolator::create(strInterp).release())
{

}

InterpolatorFun::InterpolatorFun(const PeakShape::Vector &xVals, const PeakShape::Vector &yVals, const QString &strInterp)
    :
      mPeakPosition(0.0),
      mPeakWidth(1.0),
      mPeakAmp(1.0),
      mInterp(Interpolator::create(strInterp).release())
{
    setXYValues(xVals, yVals);
}

InterpolatorFun::InterpolatorFun(const InterpolatorFun &interp)
    :
      m_vXVals(interp.m_vXVals),
      m_vYVals(interp.m_vYVals),
      mPeakPosition(interp.mPeakPosition),
      mPeakWidth(interp.mPeakWidth),
      mPeakAmp(interp.mPeakAmp),
      mInterp(Interpolator::create(interp.mInterp->name()).release())

{
}

void InterpolatorFun::setXYValues(const PeakShape::Vector &xVals, const PeakShape::Vector &yVals, bool isSorted)
{
    Q_ASSERT(xVals.size() == yVals.size());
    m_vXVals = xVals;
    m_vYVals = yVals;
    if(!isSorted)
    {
        Interpolator::pairSort(m_vXVals, m_vYVals);
    }
}

PeakShape::Type InterpolatorFun::type() const
{
    return InterpolatorFunType;
}

bool InterpolatorFun::valid() const
{
    return !m_vYVals.empty();
}

double InterpolatorFun::peakPosition() const
{
    return mPeakPosition;
}

void InterpolatorFun::setPeakPosition(double pos)
{
    mPeakPosition = pos;
}

double InterpolatorFun::peakWidth() const
{
    return mPeakWidth;
}

void InterpolatorFun::setPeakWidth(double width)
{
    mPeakWidth = width;
}

double InterpolatorFun::peakAmp() const
{
    return mPeakAmp;
}

void InterpolatorFun::setPeakAmp(double amp)
{
    mPeakAmp = amp;
}

PeakShape::Vector InterpolatorFun::values(const Vector& x) const
{
    Vector dx(x.size()), y;
    for(size_t i = 0; i < x.size(); ++i)
    {
        dx[i] = (x[i] - mPeakPosition) / mPeakWidth;
    }
    y = mInterp->interpolate(m_vXVals, m_vYVals, dx);
    for(double & yy : y) yy *= mPeakAmp;
    return y;
}

double InterpolatorFun::value(double x) const
{
    return mInterp->interpolate(m_vXVals, m_vYVals, (x - mPeakPosition)/mPeakWidth) * mPeakAmp;
}

void InterpolatorFun::import(QTextStream &out) const
{
    out.setRealNumberPrecision(10);
    out << "Amplitude:" << peakAmp() << "\n";
    out << "Width:" << mPeakWidth << "\n";
    out << "Position:" << peakPosition() << "\n";

    for(size_t i = 0; i < m_vXVals.size(); ++i)
    {
        out << m_vXVals[i] << "\t" << m_vYVals[i] << "\n";
    }
}
