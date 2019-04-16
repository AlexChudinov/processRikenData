#include "Reader.h"
#include "Base/BaseObject.h"
#include "Data/TimeEvents.h"
#include "Data/PackProc.h"

#include <QProcess>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>

Reader::Reader(QObject *parent)
    :
      QObject (parent),
      mStopFlag(false)
{

}

void Reader::stop()
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

DirectMsFromRikenTxt::DirectMsFromRikenTxt(QObject *parent)
    :
      Reader(parent),
      mFile(new QFile)
{
    connect
    (
        this,
        SIGNAL(started()),
        MyInit::instance()->timeEvents(),
        SLOT(blockingClear())
    );
}

void DirectMsFromRikenTxt::open(const QString &fileName)
{
    mFile->setFileName(fileName);
    bool fOpenned = mFile->open(QFile::ReadOnly | QFile::Text);
    Q_ASSERT(fOpenned);
}

void DirectMsFromRikenTxt::close()
{
    mFile->close();
}

void DirectMsFromRikenTxt::run()
{
    Q_EMIT started();

    MyInit::instance()->timeParams()->set
    (
        {
            {"Factor", 1},
            {"Origin", 0},
            {"Step", 1}
        }
    );

    QTextStream stream(mFile.data());
    MapIntInt ms;
    int nLines = 0;
    while(!stream.atEnd())
    {
        stream.readLine();
        nLines ++;
    }
    stream.seek(0);

    int step = nLines / 100;
    for(int i = 0; i < nLines; ++i)
    {
        quint64 chan, edge, tag, sweep;
        int evt;
        stream >> chan >> edge >> tag >> sweep >> evt;
        MapIntInt::iterator it = ms.find(evt);
        if(it != ms.end()) it->second++;
        else
        {
            ms.insert(it, {evt, 1});
        }
        if(i % step == 0) Q_EMIT progressNotify((100 * i) / nLines);
    }

    MyInit::instance()->massSpecColl()->blockingAddMassSpec(ms);

    Q_EMIT finished();
}

const char * SPAMSHexinDataX32::sWin32ProcName = "readSpams.exe";

SPAMSHexinDataX32::SPAMSHexinDataX32(QObject *parent)
    :
      Reader(parent),
      mProcess(new QProcess(this))
{
    connect
    (
        this,
        SIGNAL(started()),
        MyInit::instance()->timeEvents(),
        SLOT(blockingClear())
    );
}

void SPAMSHexinDataX32::open(const QString &fileName)
{
    mFileName = fileName;
    int res = mProcess->execute(sWin32ProcName, QStringList() << mFileName);
    if(res == 0)
    {
        QFile file("ms_out");
        file.open(QIODevice::ReadOnly);
        unsigned long long msNum;
        int nChanelNum;
        file.read(reinterpret_cast<char*>(&msNum), 8);
        file.read(reinterpret_cast<char*>(&nChanelNum), sizeof (int));
        QString item = QInputDialog::getItem
        (
            Q_NULLPTR,
            "Read Hexin data",
            "Choose data to read",
            {"Read one chanel", "Read positive", "Read negative"}
        );

        if(item == "Read one chanel")
        {
            mChanelToRead = QInputDialog::getInt
            (
                Q_NULLPTR,
                "Read Hexin data",
                "Chanel no:",
                0, 0, nChanelNum - 1
            );
            mReadState = ReadChanel;
        }
        file.close();
    }
    else
    {
        showErrMsg();
    }
}

void SPAMSHexinDataX32::close()
{
}

void SPAMSHexinDataX32::run()
{
    Q_EMIT started();

    switch(mReadState)
    {
    case ReadChanel:
        readChanel(mChanelToRead);
        break;
    }

    Q_EMIT finished();
}

void SPAMSHexinDataX32::readChanel(int nChanel) const
{
    QScopedPointer<PackProc> packer(new SimplePack<short>);
    SimplePack<short>::Header h;
    QFile file("ms_out");
    file.open(QIODevice::ReadOnly);
    quint64 nMsNum;
    qint64 pos = 0;
    int nChanelNum;
    file.read(reinterpret_cast<char*>(&nMsNum), sizeof (quint64));
    pos += sizeof (quint64);
    file.read(reinterpret_cast<char*>(&nChanelNum), sizeof (int));
    pos += sizeof (int);
    for(qulonglong i = 0; i < nMsNum; ++i)
    {
        if(i%200 == 0)
            Q_EMIT progress(static_cast<int>((i * 100) / nMsNum));
        for(int i = 0; i != nChanel; ++i)
        {
            file.read(reinterpret_cast<char*>(&h), sizeof (SimplePack<short>::Header));
            pos += h.nBytes;
            file.seek(pos);
        }

        file.read(reinterpret_cast<char*>(&h), sizeof (SimplePack<short>::Header));
        file.seek(pos);
        PackProc::DataVec data(h.nBytes);
        file.read(data.data(), h.nBytes);
        PackProc::DataVec unpackedData = packer->unpack(data);
        SimplePack<short>::Array vals
        (
            reinterpret_cast<short*>(unpackedData.data()),
            reinterpret_cast<short*>(unpackedData.data() + unpackedData.size())
        );
        pos += h.nBytes;
        file.seek(pos);
        for(int i = nChanel + 1; i < nChanelNum; ++i)
        {
            file.read(reinterpret_cast<char*>(&h), sizeof (SimplePack<short>::Header));
            pos += h.nBytes;
            file.seek(pos);
        }
        VecInt ms(vals.begin(), vals.end());
        MyInit::instance()->massSpecColl()->blockingAddMassSpec(ms);
    }
    file.close();
}

void SPAMSHexinDataX32::showErrMsg()
{
    QByteArray err = mProcess->readAllStandardError();
    QString msg(err);
    QMessageBox::warning(Q_NULLPTR, "Hexin file read", msg);
}
