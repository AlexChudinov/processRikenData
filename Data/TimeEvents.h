#ifndef TIMEEVENTS_H
#define TIMEEVENTS_H

#include <QObject>
#include <queue>
#include <mutex>

class TimeEvents : public QObject
{
    Q_OBJECT
public:
    explicit TimeEvents(QObject *parent = nullptr);

    virtual ~TimeEvents();

signals:
    void timeEventAdd();

    void timeEventsAdd(size_t);

public slots:

    void addEvent(size_t event);

private:
    std::mutex m_mutex;
    std::queue<size_t> m_timeEvents;
};

#endif // TIMEEVENTS_H
