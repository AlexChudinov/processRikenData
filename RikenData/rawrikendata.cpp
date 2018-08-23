#include "rawrikendata.h"
#include <regex>
#include <QFile>
#include <QTextStream>
#include <bitset>
#include <cmath>
#include <numeric>

RawRikenData::RawRikenData(QObject *parent)
    :
      QObject(parent)
{
}

const QStringList& RawRikenData::getStringDataHeader() const
{
    return m_strListHeader;
}

//Last and middle bit set extraction
#define LAST(k,n) ((k) & ((1<<(n))-1))
#define MID(k,m,n) LAST((k)>>(m),((n)-(m)))

void RawRikenData::readFile(QFile &file)
{
    m_strListHeader.clear();
    m_vData.clear();
    m_nMaxTime = 0;
    m_nMinTime = static_cast<quint32>(-1);

    QTextStream in(&file);
    QRegExp regExpData("\\[DATA\\]");
    QString strCurLine;
    while(!(strCurLine = in.readLine()).isNull()
          && regExpData.indexIn(strCurLine)==-1)
    {
        m_strListHeader.push_back(strCurLine);
    }

    bool ok = true;
    int sweepNumber = -1;
    quint64 count;
    while(!(strCurLine = in.readLine()).isNull() && ok)
    {
        count = strCurLine.toULongLong(&ok, 16);
        CountData countData
        {
            static_cast<short>(MID(count, 0, 3)),
            static_cast<quint32>(MID(count, 4, 32)),
            static_cast<short>(MID(count, 48, 63)),
            static_cast<bool>(MID(count, 3, 4)),
            static_cast<bool>(MID(count, 63, 64))
        };
        m_nMaxTime = qMax(m_nMaxTime, countData.time_bin);
        m_nMinTime = qMin(m_nMinTime, countData.time_bin);
        if(sweepNumber != static_cast<int>(MID(count, 32, 48)))
        {
            sweepNumber = MID(count, 32, 48);
            m_vData.push_back(CountDataCollection());
        }
        m_vData.rbegin()->push_back(countData);
    }
    m_vData.shrink_to_fit();
    for(auto& d : m_vData) d.shrink_to_fit();
}

MassSpec RawRikenData::accumulateMassSpec(size_t idx0, size_t idx1) const
{
    std::pair<size_t, size_t> pairMinMax = std::minmax(idx0, idx1);
    MassSpec::VectorInt vFreqs(m_nMaxTime - m_nMinTime + 1);
    if(pairMinMax.second <= m_vData.size())
    {
        for(size_t i = pairMinMax.first; i < pairMinMax.second; ++i)
        {
            for(const CountData& d : m_vData[i])
            {
                vFreqs[d.time_bin - m_nMinTime]++;
            }
        }
    }
    return MassSpec(m_nMinTime, std::move(vFreqs));
}

/**
 * @brief MassSpec::s_fMaxShiftValue restricts bestSqueeze function, so
 * |n/MaxTime| < s_fMaxShiftValue
 */
const double MassSpec::s_fMaxShiftValue = 1e-5;

MassSpec MassSpec::squeeze(int n) const
{
    if (n == 0) return *this;
    else
    {
        double fSqueezeFactor =
            (1. + static_cast<double>(n)
             / static_cast<double>(m_nMinTime + m_vFreqs.size() - 1));
        VectorInt vFreqs(m_vFreqs.size());
        //Calculate corresponding time scale
        std::vector<double> vTimeScale(m_vFreqs.size());
        for(size_t i = 0; i < m_vFreqs.size(); ++i)
        {
            vTimeScale[i] = (i + m_nMinTime) * fSqueezeFactor;
        }
        //For every given ref time find it correspondence
        size_t j = 0;
        for(size_t i = 0; i < m_vFreqs.size(); ++i)
        {
            double fTimeRef = static_cast<double>(i + m_nMinTime);
            while(vTimeScale[j] < fTimeRef && j < m_vFreqs.size()) ++j;
            if(j == 0) vFreqs[i] = m_vFreqs[j];
            else if (j == m_vFreqs.size()) vFreqs[i] = m_vFreqs[j-1];
            else
            {
                double w = (vTimeScale[j] - vTimeScale[j-1]);
                double w1 = (vTimeScale[j] - fTimeRef)/w;
                double w2 = (fTimeRef - vTimeScale[j-1])/w;
                    vFreqs[i] = m_vFreqs[j-1] * w1 + m_vFreqs[j] * w2;
            }
        }
        return MassSpec(m_nMinTime, vFreqs);
    }
}

int MassSpec::bestSqueeze(const MassSpec &m, bool *ok) const
{
    if(m_nMinTime != m.minTime() || m_vFreqs.size() != m.freqs().size())
    {
        if(ok) *ok = false;
        return 0;
    }
    else
    {
        //Set initial shift values
        int nl = -1, n0 = 0, nr = 1;
        VectorInt
                vl = m.squeeze(nl).freqs(),
                v0 = m.squeeze(n0).freqs(),
                vr = m.squeeze(nr).freqs();
        //Note, overflows are possible!!!
        double
                sl = std::inner_product(vl.begin(),vl.end(),m_vFreqs.begin(),0),
                s0 = std::inner_product(v0.begin(),v0.end(),m_vFreqs.begin(),0),
                sr = std::inner_product(vr.begin(),vr.end(),m_vFreqs.begin(),0);

    }
}

