#include "ParSplineCalc.h"
#include "Solvers.h"

ThreadPool::Mutex ParSplineCalc::s_mutex;
size_t ParSplineCalc::s_typeId;

ParSplineCalc::ParSplineCalc()
{
	s_typeId = registerObj("ParSplineCalc", this);
	if (!(m_threadPool = dynamic_cast<ThreadPool*>(getObj("ThreadPool"))))
	{
		throw std::runtime_error("ThreadPool should be created before ParSplineCalc.");
	}
}

ParSplineCalc::InstanceLocker ParSplineCalc::lockInstance(bool clearMemory)
{
	if (s_mutex.try_lock())
        return InstanceLocker
		(
			dynamic_cast<ParSplineCalc*>(getObj(s_typeId)),
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
    VectorDouble().swap(a);
    VectorDouble().swap(b);
    VectorDouble().swap(c);
    VectorDouble().swap(d);
    VectorDouble().swap(e);
    VectorDouble().swap(r);
    VectorDouble().swap(bb);
}

void ParSplineCalc::logSplinePoissonWeights
(
    ParSplineCalc::VectorDouble &yOut,
    const ParSplineCalc::VectorDouble &yIn,
    double p
)
{
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

	ThreadPool::FutureBunch futures
	(
		m_threadPool->parForAsync
		(
			n-2,
			[&](size_t j)->void
	{
		size_t i = j + 1;
		yOut[i] = yIn[i] < 1.0 ? p * (yIn[i] + 1.) * (yIn[i] + 1.)
			: p * (yIn[i] + 1.) * (yIn[i] + 1.) / yIn[i];
		r[i] = std::log((yIn[i - 1] + 1.) * (yIn[i + 1] + 1.)
			/ (yIn[i] + 1.) / (yIn[i] + 1.));
	}
		)
	);

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
	futures.wait();

	futures = m_threadPool->parForAsync
	(
		n-3,
		[&](size_t i)->void
	{
		a[i] = yOut[i + 1];
		b[i + 1] = -2.0*yOut[i + 1] - 2.0*yOut[i + 2] + 1.0;
		c[i + 1] = yOut[i] + 4.0*yOut[i + 1] + yOut[i + 2] + 4.0;
		d[i + 1] = -2.0*yOut[i + 1] - 2.0*yOut[i + 2] + 1.0;
		e[i + 1] = yOut[i + 2];
	}
	);
    
    //set last one value at main diagonal
    c[n-2] = yOut[n-3] + 4.0*yOut[n-2] + yOut[n-1] + 4.0;

	futures.wait();

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

    m_threadPool->parFor
    (
        n-2,
        [&](size_t j)->void
    {
		size_t i = j + 1;
        yOut[i] = std::log(yIn[i] + 1.) - (bb[i-1] - 2.0*bb[i] + bb[i+1])*yOut[i];
        yOut[i] = std::exp(yOut[i]) - 1.;
    });
}

size_t ParSplineCalc::typeId() const
{
	return s_typeId;
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
	m_instance->clear();
	if (m_clearMemory)
	{
		m_instance->freeInstance();
	}
}
