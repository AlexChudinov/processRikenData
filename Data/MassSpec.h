#ifndef MASSSPEC_H
#define MASSSPEC_H

#include <QObject>
#include <mutex>

#include "TimeEvents.h"

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
    Q_SIGNAL void massSpecsNumNotify(size_t);

    Q_SLOT void clear();
    Q_SLOT void blockingClear();

    Q_SLOT void blockingNewHist(TimeEventsContainer);
private:
    /**
     * @brief mData accumulated events histograms
     */
    HistCollection mData;

    Mutex mMutex;
};

#endif // MASSSPEC_H
