#ifndef MASSSPEC_H
#define MASSSPEC_H

#include <QObject>
#include <mutex>

class TimeEvents;

class MassSpec : public QObject
{
    Q_OBJECT
public:
    using MapUintUint = std::map<unsigned long long, unsigned long long>;
    using HistCollection = std::vector<MapUintUint>;
    using Mutex = std::mutex;
    using Locker = std::unique_lock<Mutex>;

    explicit MassSpec(QObject *parent = nullptr);

    Q_SIGNAL void cleared();
    Q_SIGNAL void changed();

    Q_SLOT void clear();
    Q_SLOT void blockingClear();
    Q_SLOT void accumulateEvents(size_t nTimeEventsCount);
    Q_SLOT void blockingAccumulateEvents(size_t nTimeEventsCount);
    Q_SLOT void setStartsPerHist(size_t startsPerHist);
    Q_SLOT void blockingSetStartsPerHist(size_t startsPerHist);
private:
    /**
     * @brief mEventsCount number of starts that were accumulated
     */
    size_t mStartsCount;

    /**
     * @brief mEventsCount number of events that were accumulated
     */
    size_t mEventsCount;

    /**
     * @brief mStartsPerHist number of starts to form one hist
     */
    size_t mStartsPerHist;

    /**
     * @brief mData accumulated events histograms
     */
    HistCollection mData;

    /**
     * @brief mEvents reference to time events object in memory
     */
    const TimeEvents * mEvents;

    Mutex mMutex;
};

#endif // MASSSPEC_H
