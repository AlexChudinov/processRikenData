#include "Reader.h"
#include "Base/BaseObject.h"
#include "Data/TimeEvents.h"

#include <QInputDialog>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>

Reader::Reader(QObject *parent)
    :
      QObject (parent)
{

}

TimeEventsReader::TimeEventsReader(QObject *parent)
    :
      Reader (parent)
{
    qRegisterMetaType<TimeEvent>("TimeEvent");

    connect(this, SIGNAL(started()),
            MyInit::instance()->timeEvents(), SLOT(blockingClear()));
    connect(this, SIGNAL(eventRead(TimeEvent)),
            MyInit::instance()->timeEvents(), SLOT(blockingAddEvent(TimeEvent)));
    connect(this, SIGNAL(finished()),
            MyInit::instance()->timeEvents(), SLOT(blockingFlushTimeSlice()));
    connect(this, SIGNAL(objPropsRead(QVariantMap)),
            MyInit::instance()->timeEvents(), SLOT(blockingAddProps(QVariantMap)));
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
    bool fOpened = mFile->open(QIODevice::ReadOnly|QIODevice::Text);
    Q_ASSERT(fOpened);
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
    Q_EMIT objPropsRead(readProps(in));

    bool ok = true;
    QString line;
    size_t prevStartIdx = 0;
    Q_EMIT eventRead(0); //add first start
    while(!(line = in.readLine()).isNull() && ok)
    {
        quint64 count = line.toULongLong(&ok, 16);
        size_t curStartIdx = MID(count, 32, 48);
        if(curStartIdx != prevStartIdx)
        {
            size_t diffStartIdx = curStartIdx > prevStartIdx ? curStartIdx - prevStartIdx :
                                                                curStartIdx + std::numeric_limits<uint16_t>::max() - prevStartIdx;
            prevStartIdx = curStartIdx;
            for(size_t i = 0; i < diffStartIdx; ++i)
                Q_EMIT eventRead(0);
        }
        Q_EMIT eventRead(MID(count, 4, 32));
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
    } while(line[0] != '[');

    in.seek(pos);
}


TxtFileReader::TxtFileReader(int scope, QObject *parent)
    : Reader(parent),
      mFolderName(),
      mScope(scope)
{
    Q_ASSERT(mScope >= 10);
    connect
    (
        this,
        SIGNAL(started()),
        MyInit::instance()->timeEvents(),
        SLOT(blockingClear())
    );
}

void TxtFileReader::open(const QString &folderName)
{
    mFolderName = folderName;
}

void TxtFileReader::close()
{
    mFolderName.clear();
}

void TxtFileReader::run()
{
    Q_EMIT started();
    QDirIterator it(mFolderName);
    QVariantMap fileNameToIdx;
    size_t idx = 0;
    while(it.hasNext())
    {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        if(fileInfo.suffix() == "txt")
        {
            QFile file(fileInfo.absoluteFilePath());
            file.open(QIODevice::ReadOnly);
            QTextStream stream(&file);
            readTimeParams(stream);
            stream.seek(0);
            readTextFile(stream);
            fileNameToIdx.insert(fileInfo.fileName(), QVariant::fromValue(idx++));
            break;
        }
    }

    double
            yMin = std::numeric_limits<double>::max(),
            yMax = std::numeric_limits<double>::min();
    while(it.hasNext())
    {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        if(fileInfo.suffix() == "txt")
        {
            QFile file(fileInfo.absoluteFilePath());
            file.open(QIODevice::ReadOnly);
            QTextStream stream(&file);
            readTextFile(stream);
            fileNameToIdx.insert(fileInfo.fileName(), QVariant::fromValue(idx++));
        }
    }

    for(DoubleVectorVector::const_reference d : mData)
    {
        yMin = qMin(yMin, *std::min_element(d.begin(), d.end()));
        yMax = qMax(yMax, *std::max_element(d.begin(), d.end()));
    }

    double factor = static_cast<double>(mScope) / (yMax - yMin);

    for(size_t i = 0; i < mData.size(); ++i)
    {
        MapUintUint ms;
        for(size_t j = 0; j < mData[i].size(); ++j)
        {
            mData[i][j] *= factor;
            ms.insert({j, static_cast<size_t>(std::round(mData[i][j]))});
        }
        MyInit::instance()->massSpec()->blockingAddMassSpec(ms);
        MyInit::instance()->massSpec()->lockInstance();
        MyInit::instance()->massSpec()->massSpecRelatedData()["file_names"]
                = fileNameToIdx;
    }

    Q_EMIT finished();
}

void TxtFileReader::readTextFile(QTextStream &stream)
{
    //Skip first line with textual information
    stream.readLine();

    if(mData.empty())
    {
        mData.push_back(DoubleVector());
    }
    else
    {
        mData.push_back(DoubleVector(mData[0].size()));
    }
    DoubleVectorVector::reverse_iterator it = mData.rbegin();
    if(it->empty())
    {
        while(!stream.atEnd())
        {
            double x, y;
            stream >> x >> y;
            if(stream.status() == QTextStream::ReadCorruptData) return;
            it->push_back(y);
        }
    }
    else
    {
        for(DoubleVector::iterator pY = it->begin(); pY != it->end(); ++pY)
        {
            double x;
            stream >> x >> *pY;
            if(stream.status() == QTextStream::ReadCorruptData) return;
        }
    }
}

void TxtFileReader::readTimeParams(QTextStream &stream)
{
    //Skip first line with textual information
    stream.readLine();

    double x0, x1;
    stream >> x0;
    stream.readLine();
    stream >> x1;

    MyInit::instance()->timeParams()->set
    (
        {
            {"Factor", x1 - x0},
            {"Origin", x0},
            {"Step", x1 - x0}
        }
    );
}



RikenDataReader::RikenDataReader(QObject *parent)
    :
      TimeEventsReader (parent),
      mFile(new QFile)
{

}

void RikenDataReader::open(const QString& fileName)
{
    mFile->setFileName(fileName);
    bool fOpened = mFile->open(QIODevice::ReadOnly|QIODevice::Text);
    Q_ASSERT(fOpened);
}

void RikenDataReader::close()
{
    mFile->close();
}

void RikenDataReader::run()
{
    Q_EMIT started();
    QTextStream stream(mFile.data());

    Q_EMIT eventRead(0);
    quint64 prevSweep = 0;
    while
    (
        stream.status() != QTextStream::ReadPastEnd
        && stream.status() != QTextStream::ReadCorruptData
    )
    {
        quint64 chan, edge, tag, sweep, evt;
        stream >> chan >> edge >> tag >> sweep >> evt;
        if(sweep != prevSweep)
        {
            quint64 diff = sweep > prevSweep ? sweep - prevSweep
                : prevSweep + std::numeric_limits<uint16_t>::max() - sweep;
            for(; diff != 0; diff--) Q_EMIT eventRead(0);
        }
        Q_EMIT eventRead(evt);
    }
    Q_EMIT finished();
}
