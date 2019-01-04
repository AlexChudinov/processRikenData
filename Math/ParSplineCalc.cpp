#include "ParSplineCalc.h"
#include "Solvers.h"
#include "Base/BaseObject.h"
#include "Base/ThreadPool.h"
#include <QMutex>

QMutex ParSplineCalc::s_mutex;

ParSplineCalc::ParSplineCalc(QObject *parent)
    :
      QObject (parent),
      a(new VectorDouble),
      b(new VectorDouble),
      c(new VectorDouble),
      d(new VectorDouble),
      e(new VectorDouble),
      bb(new VectorDouble),
      r(new VectorDouble)
{
    setObjectName("ParSplineCalc");
}

ParSplineCalc::~ParSplineCalc()
{

}

ParSplineCalc::InstanceLocker ParSplineCalc::lockInstance(bool clearMemory)
{
    if (s_mutex.tryLock())
        return InstanceLocker
		(
            MyInit::instance()->findChild<ParSplineCalc*>("ParSplineCalc"),
			clearMemory
		);
    else
        return InstanceLocker();
}

void ParSplineCalc::freeInstance()
{
    s_mutex.unlock();
}

void ParSplineCalc::clear()
{
    a.reset(new VectorDouble);
    b.reset(new VectorDouble);
    c.reset(new VectorDouble);
    d.reset(new VectorDouble);
    e.reset(new VectorDouble);
    r.reset(new VectorDouble);
    bb.reset(new VectorDouble);
}

void ParSplineCalc::logSplinePoissonWeights
(
    ParSplineCalc::VectorDouble &yOut,
    const ParSplineCalc::VectorDouble &yIn,
    double p
)
{
    const size_t n = yIn.size();
    bb->resize(n);
    r->resize(n); //right-hand side values
    (*r)[0]  = 0.0;
    (*r)[n-1]= 0.0;
    //yOut keeps weights
    yOut[0] = yIn[0] < 1.0 ? p * (yIn[0] + 1.) * (yIn[0] + 1.)
        : p * (yIn[0] + 1.) * (yIn[0] + 1.) / yIn[0];
    yOut[n-1] = yIn[n-1] < 1.0 ? p * (yIn[n-1] + 1.) * (yIn[n-1] + 1.)
        : p * (yIn[n-1] + 1.) * (yIn[n-1] + 1.) / yIn[n-1];

    ThreadPool::parFor
    (
        n-2,
        [&](size_t j)->void
    {
        const size_t i = j + 1;
        yOut[i] = yIn[i] < 1.0 ? p * (yIn[i] + 1.) * (yIn[i] + 1.)
                : p * (yIn[i] + 1.) * (yIn[i] + 1.) / yIn[i];
        (*r)[i] = std::log((yIn[i - 1] + 1.) * (yIn[i + 1] + 1.)
            / (yIn[i] + 1.) / (yIn[i] + 1.));
    }
    );

    //Allocate coefficients for a linear equations system
    a->resize(n - 2);
    b->resize(n - 1);
    c->resize(n);
    d->resize(n - 1);
    e->resize(n - 2);
    //Boundary coefficients
    (*b)[0] = -yOut[0] - 2*yOut[1] + 1.0;
    (*c)[0] = 1.0;
    (*d)[0] = 0.0;
    (*e)[0] = 0.0;
    //
    (*a)[n-3] = 0.0;
    (*b)[n-2] = 0.0;
    (*c)[n-1] = 1.0;
    (*d)[n-2] = -2*yOut[n-2] - yOut[n-1] + 1.0;
    //

    ThreadPool::parFor
    (
        n-3,
        [&](size_t i)->void
    {
        (*a)[i] = yOut[i + 1];
        (*b)[i + 1] = -2.0*yOut[i + 1] - 2.0*yOut[i + 2] + 1.0;
        (*c)[i + 1] = yOut[i] + 4.0*yOut[i + 1] + yOut[i + 2] + 4.0;
        (*d)[i + 1] = -2.0*yOut[i + 1] - 2.0*yOut[i + 2] + 1.0;
        (*e)[i + 1] = yOut[i + 2];
    }
    );

    //set last one value at main diagonal
    (*c)[n-2] = yOut[n-3] + 4.0*yOut[n-2] + yOut[n-1] + 4.0;

    math::fivediagonalsolve
    (
        static_cast<int>(n),
        a->data(),
        b->data(),
        c->data(),
        d->data(),
        e->data(),
        r->data(),
        bb->data()
    );

    yOut[0]  = std::log(yIn[0] + 1.) - ((*bb)[1]  - (*bb)[0])  * yOut[0];
    yOut[n-1]= std::log(yIn[n-1] + 1.) - ((*bb)[n-2]- (*bb)[n-1])* yOut[n-1];

    yOut[0] = std::exp(yOut[0]) - 1.;
    yOut[n-1] = std::exp(yOut[n-1]) - 1.;

    ThreadPool::parFor
    (
        n-2,
        [&](size_t j)->void
    {
        const size_t i = j + 1;
        yOut[i] = std::log(yIn[i] + 1.) - ((*bb)[i-1] - 2.0*(*bb)[i] + (*bb)[i+1])*yOut[i];
        yOut[i] = std::exp(yOut[i]) - 1.;
    });
}

ParSplineCalc::InstanceLocker::InstanceLocker
(
	ParSplineCalc * instance,
	bool clearMemory
)
	:
	m_instance(instance),
	m_clearMemory(clearMemory)
{

}

ParSplineCalc::InstanceLocker::~InstanceLocker()
{
	if (m_clearMemory)
	{
        m_instance->clear();
	}
    m_instance->freeInstance();
}
