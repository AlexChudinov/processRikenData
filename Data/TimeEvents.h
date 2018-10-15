#ifndef TIMEEVENTS_H
#define TIMEEVENTS_H

#include <QVariantMap>
#include <QObject>
#include <list>
#include <mutex>

using TimeEvent = unsigned long long;
using TimeEventsContainer = QList<TimeEvent>;

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

    Q_SLOT void blockingAddEvent(TimeEvent);
    Q_SLOT void blockingAddProps(QVariantMap);
    Q_SLOT void blockingClear();

    /**
     * @brief timeEventsSlice returns currently accumulated time slice
     * @return
     */
    TimeEventsContainer timeEventsSlice() const;

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
