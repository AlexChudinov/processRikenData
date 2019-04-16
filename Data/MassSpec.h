#ifndef MASSSPEC_H
#define MASSSPEC_H

#include <QObject>
#include <mutex>
#include <QMutex>

#include "Math/MassSpecSummator.h"
#include "TimeEvents.h"
#include "MassSpecImpl.h"

using Uint = unsigned long long;
using MapUintUint = std::map<Uint, Uint>;
using MapIntInt = std::map<int, int>;
using VecInt = std::vector<int>;
using MassSpecType = MassSpecImpl::Type;

class MassSpectrumsCollection : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameNotify)
    Q_PROPERTY(int maxBin READ maxBin)
    Q_PROPERTY(int minBin READ minBin)
    Q_PROPERTY(MassSpecType msType READ msType WRITE setMsType NOTIFY msTypeNotify)

    explicit MassSpectrumsCollection(QObject * parent = nullptr);
    virtual ~MassSpectrumsCollection();

    const QString& fileName() const;
    void setFileName(const QString &fileName);

    /**
     * @brief massSpec returns mass spectrum by idx
     * @param idx
     * @return
     */
    MassSpecImpl::MapShrdPtr massSpec(size_t idx);
    MassSpecImpl::MapShrdPtr blockingMassSpec(size_t idx);
    //Just to throw out stupid compiler warnings about type conversions
    template<typename _Number>
    MassSpecImpl::MapShrdPtr blockingMassSpec(_Number n)
    {
        Q_ASSERT(std::numeric_limits<size_t>::max() >= n && n >= 0);
        return blockingMassSpec(static_cast<size_t>(n));
    }
    /**
     * @brief unpackByMask unpacks all mass spectra for which mask[idx] is true value
     * @param mask
     */
    void unpackByMask(const std::vector<bool>& mask);
    void blockingUnpackByMask(const std::vector<bool>& mask);

    size_t size() const;
    size_t blockingSize();

    template<typename _Number> _Number blockingSize()
    {
        size_t n = blockingSize();
        Q_ASSERT(n <= std::numeric_limits<_Number>::max());
        return static_cast<_Number>(n);
    }

    int maxBin();
    int minBin();
    MassSpecType msType();
    VecInt readTotalIonCurrent(int idxFirst, int idxLast);
Q_SIGNALS:
    void cleared();
    void massSpecNumNotify(size_t);
    void timeLimitsNotify(int minTimeBin, int maxTimeBin);
    void msTypeNotify(MassSpecType);
    void fileNameNotify(QString);

public Q_SLOTS:
    void setMsType(const MassSpecType &msType);
    void clear();
    void blockingClear();
    void addMassSpec(const MapIntInt& ms);
    void blockingAddMassSpec(const MapIntInt& ms);
    void addMassSpec(TimeEventsContainer evts);
    void blockingAddMassSpec(TimeEventsContainer evts);
    void addMassSpec(const VecInt& ms);
    void blockingAddMassSpec(const VecInt& ms);
    void packAll();
    void blockingPackAll();
private:
    void checkLastTimeLimsAndNotify();
    QString mFileName;
    std::vector<MassSpecImpl*> mCollection;
    MassSpecType mMsType;
    QMutex mMut;
    //Max and min time through all mass spectra
    volatile int nMaxBin;
    volatile int nMinBin;
    friend class MSSum;
    friend class DirectSum;
};

class MassSpec : public QObject
{
    Q_OBJECT
public:
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

    void addMassSpec(const MapUintUint& ms);
    void blockingAddMassSpec(const MapUintUint& ms);

    /**
     * @brief getMassSpec
     * @param First index of first mass spectrum
     * @param Last index of last mass spectrum
     * @return accumulated mass spectrum
     */
    MapUintUint getMassSpec(size_t First, size_t Last) const;
    MapUintUint blockingGetMassSpec(size_t First, size_t Last);

    /**
     * @brief getMassSpec get mass spectrum by index
     * @param num
     * @return
     */
    const MapUintUint& getMassSpec(size_t num) const;
    MapUintUint blockingGetMassSpec(size_t num);

    /**
     * @brief getMassSpecTotalCurrent returns total number of events in mass spec
     * @param idx
     * @return
     */
    double getMassSpecTotalCurrent(size_t idx) const;
    double blockingGetMassSpecTotalCurrent(size_t idx);

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

    /**
     * @brief massSpecRelatedData get data from mass spectra
     * @return
     */
    const QVariantMap& massSpecRelatedData() const;
    QVariantMap& massSpecRelatedData();

    /**
     * @brief lockInstance locks current mass spectrum instance
     * @return
     */
    Locker lockInstance();
private:
    /**
     * @brief mMassSpecRelatedData
     * Some mass spec data
     */
    QVariantMap mMassSpecRelatedData;

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

    /**
     * @brief mSummator interface for summing mass spectrums together
     */
    QScopedPointer<MassSpecSummator> mSummator;
};

#endif // MASSSPEC_H
