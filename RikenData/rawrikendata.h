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

    MassSpec(quint8 nMinTime, const VectorInt& vAmplitudes)
        :
          m_nMinTime(nMinTime),
          m_vAmplitudes(vAmplitudes)
    {}

    MassSpec(quint8 nMinTime, VectorInt&& vAmplitudes)
        :
          m_nMinTime(nMinTime),
          m_vAmplitudes(vAmplitudes)
    {}

private:
    quint32 m_nMinTime;
    std::vector<quint32> m_vAmplitudes;
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
