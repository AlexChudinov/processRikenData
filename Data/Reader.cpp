#include "Reader.h"
#include "Base/BaseObject.h"
#include "Data/TimeEvents.h"
#include <QFile>
#include <QTextStream>

Reader::Reader(QObject *parent)
    :
      QObject (parent)
{

}

TimeEventsReader::TimeEventsReader(QObject *parent)
    :
      Reader (parent)
{
    Q_ASSERT(mTimeEvents = MyInit::instance().findChild<TimeEvents*>("TimeEvents"));
}


RikenFileReader::RikenFileReader(QObject *parent)
    :
      TimeEventsReader (parent),
      mFile(new QFile)
{

}

void RikenFileReader::open(const QString &fileName)
{
    mFile->setFileName(fileName);
    Q_ASSERT(mFile->open(QIODevice::ReadOnly|QIODevice::Text));
}

void RikenFileReader::close()
{
    mFile->close();
}

//Last and middle bit set extraction
#define LAST(k,n) ((k) & ((1<<(n))-1))
#define MID(k,m,n) LAST((k)>>(m),((n)-(m)))
void RikenFileReader::run()
{
    Q_EMIT started();
    QTextStream in(mFile.data());
    mTimeEvents->blockingAddProps(readProps(in));

    bool ok = true;
    QString line;
    size_t prevStartIdx = 0;
    mTimeEvents->blockingAddEvent(0); //add first start
    while(!(line = in.readLine()).isNull() && ok)
    {
        quint64 count = line.toULongLong(&ok, 16);
        size_t curStartIdx = MID(count, 32, 48);
        if(curStartIdx != prevStartIdx)
        {
            size_t diffStartIdx = curStartIdx >= prevStartIdx ? curStartIdx - prevStartIdx :
                                                                curStartIdx + std::numeric_limits<uint16_t>::max() - prevStartIdx;
            prevStartIdx = curStartIdx;
            for(size_t i = 0; i < diffStartIdx; ++i)
                mTimeEvents->blockingAddEvent(0);
        }
        mTimeEvents->blockingAddEvent(MID(count, 4, 32));
    }
    Q_EMIT finished();
}

QVariantMap RikenFileReader::readProps(QTextStream &in)
{
    QVariantMap result;

    //Read segment name
    QString segName = in.readLine();
    while(segName != "[DATA]")
    {
        QVariant& seg = result[segName];
        QVariantMap segProps;
        readPropsSegment(in, segProps);
        seg = segProps;
        segName = in.readLine();
    }

    return result;
}

void RikenFileReader::readPropsSegment(QTextStream &in, QVariantMap &seg)
{
    QString line;
    qint64 pos;

    do
    {
        pos = in.pos();
        line = in.readLine();
        if(line.contains('='))
        {
            QStringList list = line.split("=");
            seg[list[0]] = list[1];
        }
    } while(line[0] != "[");

    in.seek(pos);
}
