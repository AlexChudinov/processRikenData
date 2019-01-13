#ifndef MATH_OBJ_H
#define MATH_OBJ_H

#include <QThread>
#include <QVariant>
#include <string>
#include <vector>
#include <map>

//Initialises objects
class TimeEvents;
class MassSpec;
class TimeParams;
class MyInit : public QObject
{
    Q_OBJECT

    static MyInit * s_instance;

public:
    MyInit(QObject * parent = nullptr);
	~MyInit();

    static MyInit * instance();

    TimeEvents * timeEvents();

    MassSpec * massSpec();

    TimeParams * timeParams();

    void moveToThread(QObject * obj);
private:
    QScopedPointer<TimeEvents> mTimeEvents;
    QScopedPointer<MassSpec> mMassSpec;
    QScopedPointer<TimeParams> mTimeParams;
};

class SimpleThread : public QThread
{
public:
    void run();
};

#endif
