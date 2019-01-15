#include "Reader.h"
#include "Base/BaseObject.h"
#include "Data/TimeEvents.h"

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


TxtFileReader::TxtFileReader(QObject *parent)
    : Reader(parent),
      mFolderName()
{
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
    while(it.hasNext())
    {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        if(fileInfo.suffix() == "txt")
        {
            QFile file(fileInfo.absoluteFilePath());
            file.open(QIODevice::ReadOnly);
            QTextStream stream(&file);
            MyInit::instance()->massSpec()->blockingAddMassSpec
            (
                readTextFile(stream)
            );
        }
    }
    Q_EMIT finished();
}

MapUintUint TxtFileReader::readTextFile(QTextStream &stream)
{
    MapUintUint MS;

    //Skip first text line
    stream.readLine();

    QVector<double> x, y;
    while(!stream.atEnd())
    {
        QString line = stream.readLine();
        QTextStream lineStream(&line);
        double xx, yy;
        lineStream >> xx >> yy;
        if(lineStream.status() == QTextStream::ReadCorruptData)
        {
            return MS;
        }
        x.push_back(xx); y.push_back(yy);
    }

    MyInit::instance()->timeParams()->set
    (
        {
            {"Factor", x[1] - x[0]},
            {"Origin", x[0]},
            {"Step", x[1] - x[0]}
        }
    );

    size_t idxPrev = 0;
    for(int i = 0; i < x.size(); ++i)
    {
        size_t idxCur = static_cast<size_t>(qRound((x[i] - x[0]) / (x[1] - x[0])));
        if(idxCur != 0 && idxCur - idxPrev != 1)
        {
            return MapUintUint();
        }
        MS.insert({idxCur, y[i]});
        idxPrev = idxCur;
    }

    return MS;
}


