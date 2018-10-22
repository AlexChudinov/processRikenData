#ifndef MASSSPEC_H
#define MASSSPEC_H

#include <QObject>
#include <mutex>

#include "TimeEvents.h"

class MassSpec : public QObject
{
    Q_OBJECT
public:
    using Uint = unsigned long long;
    using MapUintUint = std::map<Uint, Uint>;
    using VectorUint = std::vector<Uint>;
    using HistCollection = std::vector<MapUintUint>;
    using Mutex = std::mutex;
    using Locker = std::unique_lock<Mutex>;

    explicit MassSpec(QObject *parent = nullptr);

    Q_SIGNAL void cleared();
    Q_SIGNAL void massSpecsNumNotify(size_t);
    Q_SIGNAL void timeLimitsNotify(Uint minTimeBin, Uint maxTimeBin);

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
    VectorUint getIonCurrent(Uint First, Uint Last) const;
    VectorUint blockingGetIonCurrent(Uint First, Uint Last);

    /**
     * @brief updateLastMS returns last mass spectrum
     * @return
     */
    MapUintUint lastMS() const;
    MapUintUint blockingLastMS();
    /**
     * @brief updateLastTic returns TIC in the last mass spectrum
     * @return
     */
    double lastTic(Uint First, Uint Last) const;
    double blockingLastTic(Uint First, Uint Last);

    /**
     * @brief size returns size of array of mass specs
     * @return
     */
    size_t size() const;
    size_t blockingSize();

    std::pair<Uint, Uint> minMaxTime() const;
    std::pair<Uint, Uint> blockingMinMaxTime();
private:
    /**
     * @brief mData accumulated events histograms
     */
    HistCollection mData;

    /**
     * @brief mMinTimeBin minimal time bin in histogram data
     */
    Uint mMinTimeBin;

    /**
     * @brief mMaxTimeBin maximal time bin in histogram data
     */
    Uint mMaxTimeBin;

    Mutex mMutex;
};

#endif // MASSSPEC_H
