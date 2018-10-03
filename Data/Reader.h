#pragma once
#ifndef TIME_EVENTS_H
#define TIME_EVENTS_H
#include <QObject>
#include <vector>
#include <map>

/**
	@class TimeEventsReader is an interface to read time events from file or etc.
*/
class Reader: public QObject
{
	Q_OBJECT

public:
	Reader(QObject * parent = Q_NULLPTR);

	virtual ~Reader() {}

    virtual void open(const QString& fileName) = 0;

    virtual void close() = 0;

    virtual void run() = 0;
};

#endif // !TIME_EVENTS_H


