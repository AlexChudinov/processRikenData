#include "BaseObject.h"
#include "Math/ParSplineCalc.h"
#include "Data/TimeEvents.h"
#include "Data/MassSpec.h"
#include "Math/MassSpecSummator.h"

MyInit * MyInit::s_instance;

MyInit::MyInit(QObject * parent)
    :
      QObject(parent),
      mRealNumPrecision(6)
{
    qRegisterMetaType<size_t>("size_t");
    if(s_instance) throw std::runtime_error
            ("Tryed to create second MyInit instance!");
    s_instance = this;
    new ParSplineCalc(this);
    mMassSpec.reset(new MassSpec);
    mTimeEvents.reset(new TimeEvents);
    mTimeParams.reset(new TimeParams);
    mMassSpecsColl.reset(new MassSpectrumsCollection);

    moveToThread(massSpecColl());
    moveToThread(massSpec());
    moveToThread(timeEvents());

    /*connect(timeEvents(), SIGNAL(sliceAccumulated(TimeEventsContainer)),
            massSpec(), SLOT(blockingNewHist(TimeEventsContainer)));

    connect(timeEvents(), SIGNAL(cleared()),
            massSpec(), SLOT(blockingClear()));*/

    connect
    (
        timeEvents(),
        SIGNAL(sliceAccumulated(TimeEventsContainer)),
        massSpecColl(),
        SLOT(blockingAddMassSpec(TimeEventsContainer))
    );

    connect
    (
        timeEvents(),
        SIGNAL(cleared()),
        massSpecColl(),
        SLOT(blockingClear())
    );
}

MyInit::~MyInit()
{
}

MyInit * MyInit::instance()
{
    return s_instance;
}

TimeEvents * MyInit::timeEvents()
{
    return mTimeEvents.data();
}

MassSpec * MyInit::massSpec()
{
    return mMassSpec.data();
}

TimeParams *MyInit::timeParams()
{
    return mTimeParams.data();
}

MassSpectrumsCollection *MyInit::massSpecColl()
{
    return mMassSpecsColl.data();
}

int MyInit::precision()
{
    return mRealNumPrecision;
}

void MyInit::setPrecision(int prec)
{
    mRealNumPrecision = prec;
    Q_EMIT precisionNotify(prec);
}

void MyInit::moveToThread(QObject *obj)
{
    SimpleThread * thread = new SimpleThread;
    connect(this, SIGNAL(destroyed()), thread, SLOT(deleteLater()));
    obj->moveToThread(thread);
    thread->start();
}

void SimpleThread::run()
{
    QThread::exec();
}
