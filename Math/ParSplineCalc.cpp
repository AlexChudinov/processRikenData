#include "ParSplineCalc.h"
#include "Solvers.h"

ParSplineCalc ParSplineCalc::s_instance;

QMutex ParSplineCalc::s_mutex;

ParSplineCalc::InstanceLocker ParSplineCalc::lockInstance()
{
    if(s_mutex.tryLock())
        return InstanceLocker(&s_instance);
    else
        return InstanceLocker();
}

void ParSplineCalc::freeInstance()
{
    s_mutex.unlock();
}

void ParSplineCalc::clear()
{
    a.clear();
    b.clear();
    c.clear();
    d.clear();
    e.clear();
    r.clear();
    bb.clear();
}

void ParSplineCalc::logSplinePoissonWeights
(
    ParSplineCalc::VectorDouble &yOut,
    const ParSplineCalc::VectorDouble &yIn,
    double p
)
{
    using VectorIdx = std::vector<size_t>;
    const size_t n = yIn.size();
    bb.resize(n);
    r.resize(n); //right-hand side values
    r[0]  = 0.0;
    r[n-1]= 0.0;
    //yOut keeps weights
    yOut[0] = yIn[0] < 1.0 ? p * (yIn[0] + 1.) * (yIn[0] + 1.)
        : p * (yIn[0] + 1.) * (yIn[0] + 1.) / yIn[0];
    yOut[n-1] = yIn[n-1] < 1.0 ? p * (yIn[n-1] + 1.) * (yIn[n-1] + 1.)
        : p * (yIn[n-1] + 1.) * (yIn[n-1] + 1.) / yIn[n-1];

    VectorIdx Idx(n);
    std::iota(Idx.begin(), Idx.end(), 0);

    QtConcurrent::map<VectorIdx::iterator>
    (
        std::next(Idx.begin()),
        std::prev(Idx.end()),
        [&](size_t i)->void
    {
        yOut[i] = yIn[i] < 1.0 ? p * (yIn[i] + 1.) * (yIn[i] + 1.)
            : p * (yIn[i] + 1.) * (yIn[i] + 1.) / yIn[i];
        r[i] = std::log((yIn[i-1] + 1.) * (yIn[i+1] + 1.)
                / (yIn[i] + 1.) / (yIn[i] + 1.));
    }).waitForFinished();

    //Allocate coefficients for a linear equations system
    a.resize(n - 2);
    b.resize(n - 1);
    c.resize(n);
    d.resize(n - 1);
    e.resize(n - 2);
    //Boundary coefficients
    b[0] = -yOut[0] - 2*yOut[1] + 1.0;
    c[0] = 1.0;
    d[0] = 0.0;
    e[0] = 0.0;
    //
    a[n-3] = 0.0;
    b[n-2] = 0.0;
    c[n-1] = 1.0;
    d[n-2] = -2*yOut[n-2] - yOut[n-1] + 1.0;
    //
    QtConcurrent::map<VectorIdx::iterator>
    (
        Idx.begin(),
        Idx.end() - 3,
        [&](size_t i)->void
    {
        a[i]   = yOut[i+1];
        b[i+1] = -2.0*yOut[i+1] - 2.0*yOut[i+2] + 1.0;
        c[i+1] = yOut[i] + 4.0*yOut[i+1] + yOut[i+2] + 4.0;
        d[i+1] = -2.0*yOut[i+1] - 2.0*yOut[i+2] + 1.0;
        e[i+1] = yOut[i+2];
    }).waitForFinished();

    //set last one value at main diagonal
    c[n-2] = yOut[n-3] + 4.0*yOut[n-2] + yOut[n-1] + 4.0;

    math::fivediagonalsolve
    (
        static_cast<int>(n),
        a.data(),
        b.data(),
        c.data(),
        d.data(),
        e.data(),
        r.data(),
        bb.data()
    );

    yOut[0]  = std::log(yIn[0] + 1.) - (bb[1]  - bb[0])  * yOut[0];
    yOut[n-1]= std::log(yIn[n-1] + 1.) - (bb[n-2]- bb[n-1])* yOut[n-1];

    yOut[0] = std::exp(yOut[0]) - 1.;
    yOut[n-1] = std::exp(yOut[n-1]) - 1.;

    QtConcurrent::map<VectorIdx::iterator>
    (
        std::next(Idx.begin()),
        std::prev(Idx.end()),
        [&](size_t i)->void
    {
        yOut[i] = std::log(yIn[i] + 1.) - (bb[i-1] - 2.0*bb[i] + bb[i+1])*yOut[i];
        yOut[i] = std::exp(yOut[i]) - 1.;
    }).waitForFinished();
}
