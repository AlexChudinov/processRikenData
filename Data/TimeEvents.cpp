#include "Data/MassSpec.h"
#include "Base/BaseObject.h"
#include "TimeEvents.h"

TimeEvents::TimeEvents(QObject *parent)
    :
      QObject(parent),
      mStartsPerHist(1000),
      mStartsCount(0)
{
    qRegisterMetaType<TimeEventsContainer>("TimeEventsContainer");
    setObjectName("TimeEvents");
}

TimeEvents::~TimeEvents()
{

}

void TimeEvents::blockingAddEvent(TimeEvent evt)
{
    Locker lock(mMutex);

    if(!evt && mStartsCount++ == mStartsPerHist)
    {
        mStartsCount = 1;
        flushTimeSlice();
        mTimeEventsSlice.push_back(evt);
    }
    else
    {
        mTimeEventsSlice.push_back(evt);
    }
}

void TimeEvents::blockingAddProps(QVariantMap props)
{
    Locker lock(mMutex);
    mProps.reset(new QVariantMap(props));
    Q_EMIT propsUpdateNotify();
}

void TimeEvents::blockingClear()
{
    Locker lock(mMutex);
    mStartsCount = 0;
    mTimeEvents.clear();
    mTimeEventsSlice.clear();
    Q_EMIT cleared();
}

void TimeEvents::flushTimeSlice()
{
    mTimeEvents.append(mTimeEventsSlice);
    Q_EMIT eventsUpdateNotify(static_cast<size_t>(mTimeEvents.size()));
    Q_EMIT sliceAccumulated(mTimeEventsSlice);
    mTimeEventsSlice.clear();
}



void TimeEvents::blockingFlushTimeSlice()
{
    Locker lock(mMutex);
    flushTimeSlice();
}

void TimeEvents::recalculateTimeSlices(size_t startsPerHist)
{
    Q_EMIT beforeRecalculation();
    Locker lock(mMutex);
    if(!mTimeEvents.empty() && startsPerHist != 0 && mStartsPerHist != startsPerHist)
    {
        MyInit::instance()->massSpec()->blockingClear();
        mStartsPerHist = startsPerHist;
        mStartsCount = 0;
        mTimeEventsSlice.clear();

        for(TimeEvent evt : mTimeEvents)
        {
            if(!evt && mStartsCount++ == mStartsPerHist)
            {
                mStartsCount = 1;
                Q_EMIT sliceAccumulated(mTimeEventsSlice);
                mTimeEventsSlice.clear();
                mTimeEventsSlice.push_back(evt);
            }
            else
            {
                mTimeEventsSlice.push_back(evt);
            }
        }
        if(mTimeEventsSlice.size() != 1) Q_EMIT sliceAccumulated(mTimeEventsSlice);
    }
    Q_EMIT recalculated();
}

size_t TimeEvents::startsPerHist()
{
    Locker lock(mMutex);
    return mStartsPerHist;
}

TimeParams::TimeParams(QObject *parent)
    :
      QObject(parent),
      mTimeFactor(1),
      mTimeStep(1),
      mTimeOrigin(0),
      mTimeUnits(QObject::tr("bin"))
{
    setObjectName("TimeParams");
}

const QStringList &TimeParams::parameters() const
{
    static const QStringList s_parameters {"Factor", "Origin", "Step", "Units"};
    return s_parameters;
}

QVariantMap TimeParams::get() const
{
    return QVariantMap
    {
        {parameters()[0], QVariant(mTimeFactor)},
        {parameters()[1], QVariant(mTimeOrigin)},
        {parameters()[2], QVariant(mTimeStep)},
        {parameters()[3], QVariant(mTimeUnits)}
    };
}

void TimeParams::set(const QVariantMap &params)
{
    for(const QString& name : parameters())
    {
        QVariantMap::ConstIterator it = params.find(name);
        if (it != params.end())
        {
            setByName(name, it.value());
        }
    }
    Q_EMIT setParamsNotify();
}

void TimeParams::setByName(const QString &name, QVariant val)
{
    if(name == parameters()[0] && val.canConvert<double>())
    {
        mTimeFactor = val.toDouble();
    }
    else if(name == parameters()[1] && val.canConvert<double>())
    {
        mTimeOrigin = val.toDouble();
    }
    else if(name == parameters()[2] && val.canConvert<double>())
    {
        mTimeStep = val.toDouble();
    }
    else if(name == parameters()[3] && val.canConvert<QString>())
    {
        mTimeUnits = val.toString();
    }
}
