#ifndef RAWRIKENDATA_H
#define RAWRIKENDATA_H

#include <QObject>
#include <QVariant>
#include <vector>

class QTextStream;
class QFile;

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



private:
    QStringList m_strListHeader;
    quint32 m_nMaxTime;
    quint32 m_nMinTime;
    std::vector<CountDataCollection> m_vData;
};

#endif // RAWRIKENDATA_H
