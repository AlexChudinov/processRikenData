#include "PeakShape.h"
#include <QStringList>

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
    for(double& x : m_vXVals)
    {
        x -= mPeakPosition;
        x += pos;
    }
    mPeakPosition = pos;
}

double InterpolatorFun::peakWidth() const
{
    return mPeakWidth;
}

void InterpolatorFun::setPeakWidth(double width)
{
    for(double& x : m_vXVals)
    {
        double dx = (x - mPeakPosition) * width / mPeakWidth;
        x = mPeakPosition + dx;
    }
    mPeakWidth = width;
}

double InterpolatorFun::peakAmp() const
{
    return mPeakAmp;
}

void InterpolatorFun::setPeakAmp(double amp)
{
    for(double& y : m_vYVals)
    {
        y *= amp / mPeakAmp;
    }
    mPeakAmp = amp;
}

PeakShape::Vector InterpolatorFun::values(const Vector& x) const
{
    return mInterp->interpolate(m_vXVals, m_vYVals, x);
}
