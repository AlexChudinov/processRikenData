#ifndef TIMEEVENTS_H
#define TIMEEVENTS_H

#include <QVariantMap>
#include <QObject>
#include <list>
#include <mutex>

using TimeEvent = unsigned long long;
using TimeEventsContainer = QList<TimeEvent>;

/**
 * @brief The TimeParams class keeps parameters of time events
 * to transform numbers to a real time units
 */
class TimeParams : public QObject
{
    Q_OBJECT
public:

    TimeParams(QObject * parent = Q_NULLPTR);

    const QStringList& parameters() const;

    QVariantMap get() const;

    void set(const QVariantMap& params);

signals:

    void setParamsNotify();

private:
    double mTimeFactor; //time unit
    double mTimeStep;   //one time step
    double mTimeOrigin; //time values origin
    QString mTimeUnits;

    void setByName(const QString& name, QVariant val);

    friend class TimeScale;
};

class TimeEvents : public QObject
{
    Q_OBJECT
public:
    using Mutex = std::mutex;
    using Locker = std::unique_lock<Mutex>;

    explicit TimeEvents(QObject *parent = nullptr);

    virtual ~TimeEvents();

    /**
     * @brief events
     * @return reference to read events
     */
    const TimeEventsContainer& events() const { return mTimeEvents; }

    /**
     * @brief props
     * @return reference to downloaded properties
     */
    const QVariantMap& props() const { return *mProps; }

    Q_SIGNAL void eventsUpdateNotify(size_t);
    Q_SIGNAL void propsUpdateNotify();
    Q_SIGNAL void cleared();
    Q_SIGNAL void sliceAccumulated(TimeEventsContainer);
    Q_SIGNAL void beforeRecalculation();
    Q_SIGNAL void recalculated();

    Q_SLOT void blockingAddEvent(TimeEvent);
    Q_SLOT void blockingAddProps(QVariantMap);
    Q_SLOT void blockingClear();
    Q_SLOT void flushTimeSlice();
    Q_SLOT void blockingFlushTimeSlice();

    /**
     * @brief recalculateTimeSlices sets new starts count for accumulation and recalculates time slices
     */
    Q_SLOT void recalculateTimeSlices(size_t);

    size_t startsPerHist();
private:

    Mutex mMutex;

    TimeEventsContainer mTimeEvents;
    /**
     * @brief mTimeEventsSlice accumulation of time events for  mStartsPerHist
     */
    TimeEventsContainer mTimeEventsSlice;

    QScopedPointer<QVariantMap> mProps;

    size_t mStartsPerHist;
    size_t mStartsCount;
};

#endif // TIMEEVENTS_H
