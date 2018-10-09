#ifndef TIMEEVENTS_H
#define TIMEEVENTS_H

#include <QVariantMap>
#include <QObject>
#include <list>
#include <mutex>

class TimeEvents : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief The TimeEvent struct particular time event
     */
    struct TimeEvent
    {
        size_t start;
        size_t stop;
    };

    using TimeEventsContainer = std::list<TimeEvent>;
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

    Q_SLOT void blockingAddEvent(size_t start, size_t stop);
    Q_SLOT void blockingAddProps(QVariantMap props);
private:

    Mutex mMutex;

    TimeEventsContainer mTimeEvents;

    QScopedPointer<QVariantMap> mProps;
};

#endif // TIMEEVENTS_H
