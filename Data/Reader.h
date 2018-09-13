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

protected:

};

#endif // !TIME_EVENTS_H


