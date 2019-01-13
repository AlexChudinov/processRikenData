#include "BaseObject.h"
#include "Math/ParSplineCalc.h"
#include "Data/TimeEvents.h"
#include "Data/MassSpec.h"

MyInit * MyInit::s_instance;

MyInit::MyInit(QObject * parent)
    :
      QObject(parent)
{
    qRegisterMetaType<size_t>("size_t");
    if(s_instance) throw std::runtime_error
            ("Tryed to create second MyInit instance!");
    s_instance = this;
    new ParSplineCalc(this);
    mMassSpec.reset(new MassSpec);
    mTimeEvents.reset(new TimeEvents);
    mTimeParams.reset(new TimeParams);

    moveToThread(massSpec());
    moveToThread(timeEvents());

    connect(timeEvents(), SIGNAL(sliceAccumulated(TimeEventsContainer)),
            massSpec(), SLOT(blockingNewHist(TimeEventsContainer)));

    connect(timeEvents(), SIGNAL(cleared()),
            massSpec(), SLOT(blockingClear()));
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
