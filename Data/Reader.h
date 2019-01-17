#pragma once
#ifndef TIME_EVENTS_H
#define TIME_EVENTS_H
#include <QFile>
#include <QObject>
#include <QRunnable>
#include <vector>
#include <map>

#include "Data/TimeEvents.h"
#include "Data/MassSpec.h"

/**
	@class TimeEventsReader is an interface to read time events from file or etc.
*/
class Reader: public QObject, public QRunnable
{
	Q_OBJECT

public:
	Reader(QObject * parent = Q_NULLPTR);

	virtual ~Reader() {}

    virtual void open(const QString& fileName) = 0;

    virtual void close() = 0;

    Q_SIGNAL void objPropsRead(QVariantMap);
    Q_SIGNAL void started();
    Q_SIGNAL void finished();
};

class QFile;
class TimeEvents;
class QTextStream;

class TimeEventsReader : public Reader
{
    Q_OBJECT

public:
    TimeEventsReader(QObject * parent = Q_NULLPTR);

    Q_SIGNAL void eventRead(TimeEvent evt);
};

class RikenFileReader : public TimeEventsReader
{
    Q_OBJECT

public:
    RikenFileReader(QObject * parent = Q_NULLPTR);

    void open(const QString& fileName);

    void close();

    void run();

private:
    QScopedPointer<QFile> mFile;

    static QVariantMap readProps(QTextStream& in);
    static void readPropsSegment(QTextStream& in, QVariantMap& seg);
};

class TxtFileReader : public Reader
{
    Q_OBJECT

public:
    using DoubleVector = std::vector<double>;
    using DoubleVectorVector = std::vector<DoubleVector>;

    TxtFileReader(int scope, QObject * parent = Q_NULLPTR);

    void open(const QString& folderName);

    void close();

    void run();

    Q_SIGNAL void massSpectrumReadNotify(MapUintUint);
private:
    QString mFolderName;

    int mScope;

    //Already read data
    DoubleVectorVector mData;

    //Reads second data column in stream and stores it to mData
    void readTextFile(QTextStream & stream);

    //Reads first file and first column in it to estimate
    //time params.
    void readTimeParams(QTextStream & stream);
};

#endif // !TIME_EVENTS_H


