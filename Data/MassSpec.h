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

    /**
     * @brief getMassSpec
     * @param First index of first mass spectrum
     * @param Last index of last mass spectrum
     * @return accumulated mass spectrum
     */
    MapUintUint getMassSpec(size_t First, size_t Last) const;
    MapUintUint blockingGetMassSpec(size_t First, size_t Last);

    /**
     * @brief getIonCurrent
     * @param First first time bin value
     * @param Last last time bin value
     * @return ion current inside chosen time bins
     */
    MapUintUint getIonCurrent(size_t First, size_t Last) const;
    MapUintUint blockingGetIonCurrent(size_t First, size_t Last);

    /**
     * @brief size returns size of array of mass specs
     * @return
     */
    size_t size() const;
    size_t blockingSize();

    unsigned long long maxTime() const;
    unsigned long long blockingMaxTime();
private:
    /**
     * @brief mData accumulated events histograms
     */
    HistCollection mData;

    Mutex mMutex;
};

#endif // MASSSPEC_H
