#include "rawrikendata.h"
#include <regex>
#include <QFile>
#include <QTextStream>
#include <bitset>

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
}
