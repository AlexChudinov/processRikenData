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

AlglibInterpolator::AlglibInterpolator(const PeakShape::Vector &x, const PeakShape::Vector &y, int M)
    :
      mShape(new alglib::spline1dinterpolant)
{
    Q_ASSERT(x.size() == y.size());
    alglib::ae_int_t N = static_cast<alglib::ae_int_t>(x.size());
    alglib::real_1d_array xx, yy, ww;
    xx.setlength(N);
    yy.setlength(N);
    ww.setlength(N);
    xx.setcontent(N, x.data());
    yy.setcontent(N, y.data());
    for(int i = 0; i < N; ++i)
    {
        ww[i] = std::sqrt(y[i] + 1.);
        yy[i] = std::log(y[i] + 1.);
    }

    auto chiSquare = [&](double rho)->double
    {
        alglib::ae_int_t info;
        alglib::spline1dfitreport rep;
        alglib::spline1dfitpenalizedw(xx, yy, ww, N, M, rho, info, *mShape, rep);
        int NN = 0;
        double s = 0.;
        Vector yyy = values(x);
        for(size_t i = 0; i < static_cast<size_t>(N); ++i)
        {
            if(yyy[i] > 0.)
            {
                double dy = yyy[i] - y[i];
                s += dy * dy / yyy[i];
                NN++;
            }
        }
        return s/NN - 1.;
    };

    double rho = math::froot(chiSquare, -8., 8.);

    alglib::ae_int_t info;
    alglib::spline1dfitreport rep;
    alglib::spline1dfitpenalizedw(xx, yy, ww, N, M, rho, info, *mShape, rep);
}

PeakShape::Type AlglibInterpolator::type() const
{
    return PeakShape::AlglibInterpolatorType;
}

bool AlglibInterpolator::valid() const
{
    return bool(mShape);
}

PeakShape::Vector AlglibInterpolator::values(const PeakShape::Vector &x) const
{
    Vector y(x.size());
    for(size_t i = 0; i < x.size(); ++i)
    {
        double dx = (x[i] - mPeakPosition) / mPeakWidth;
        y[i] = (std::exp(alglib::spline1dcalc(*mShape, dx)) - 1.) * mPeakAmp;
    }
    return y;
}
