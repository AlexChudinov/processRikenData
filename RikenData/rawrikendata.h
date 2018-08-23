#ifndef RAWRIKENDATA_H
#define RAWRIKENDATA_H

#include <QObject>
#include <QVariant>
#include <vector>

class QTextStream;
class QFile;

/**
 * @brief The MassSpec class implements histogram of counts
 */
class MassSpec
{
public:
    using VectorInt = std::vector<quint32>;

    MassSpec(quint32 nMinTime, const VectorInt& vFreqs)
        :
          m_nMinTime(nMinTime),
          m_vFreqs(vFreqs)
    {}

    MassSpec(quint32 nMinTime, VectorInt&& vFreqs)
        :
          m_nMinTime(nMinTime),
          m_vFreqs(vFreqs)
    {}

    inline quint32 minTime() const { return m_nMinTime; }
    inline const VectorInt& freqs() const { return m_vFreqs; }

    /**
     * @brief squeeze squeezes or stretches original mass spec
     * @param n number of time bines
     * @return squeezed or stretched mass spec
     */
    MassSpec squeeze(int n) const;

    /**
     * @brief bestSqueeze looks for best squeeze value using this as a
     * reference one
     * @param m
     * @param ok The routine checks inside that sizes of mass spectrums
     * and their minimum time values are the same. This case it calculates
     * the time shift and returns one putting "ok" to true otherwise it does
     * early return of zero value for time shift and puts "ok" to false. False
     * flag also will be set up if the shift value went out the limits
     * @return value of a time shift where the mass specs is matching
     */
    int bestSqueeze(const MassSpec& m, bool *ok) const;

private:
    quint32 m_nMinTime;
    VectorInt m_vFreqs;

    static const double s_fMaxShiftValue;
};

/**
 * @brief The RawRikenData class is binary representation of a .lst file
 */
class RawRikenData : public QObject
{
    Q_OBJECT
public:
    explicit RawRikenData(QObject *parent = 0);

    /**
     * @brief The CountData struct keeps information about one ion count
     */
    struct CountData
    {
        short chan;
        quint32 time_bin;
        short tag;
        bool edge;
        bool data_lost;
    };
    using CountDataCollection = std::vector<CountData>;

    const QStringList &getStringDataHeader() const;

    /**
     * @brief readFile reads data from a file
     * @param file
     */
    void readFile(QFile& file);

    inline quint32 maxTime() const { return m_nMaxTime; }
    inline quint32 minTime() const { return m_nMinTime; }
    inline size_t sweepsNumber() const { return m_vData.size(); }

    /**
     * @brief accumulateMassSpec accumulates data from the counts
     * inside given sweep indices
     * @param idx0 first sweep index
     * @param idx1 last sweep index
     * @return
     */
    MassSpec accumulateMassSpec(size_t idx0, size_t idx1) const;

private:
    QStringList m_strListHeader;
    quint32 m_nMaxTime;
    quint32 m_nMinTime;
    std::vector<CountDataCollection> m_vData;
};

#endif // RAWRIKENDATA_H
