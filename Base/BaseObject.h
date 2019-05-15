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
class MassSpectrumsCollection;
class MyInit : public QObject
{
    Q_OBJECT

    static MyInit * s_instance;

    Q_PROPERTY(int mRealNumPrecision READ precision WRITE setPrecision NOTIFY precisionNotify)

public:
    MyInit(QObject * parent = nullptr);
	~MyInit();

    static MyInit * instance();

    TimeEvents * timeEvents();

    MassSpec * massSpec();

    TimeParams * timeParams();

    MassSpectrumsCollection * massSpecColl();

    int precision();
    void setPrecision(int prec);
    Q_SIGNAL void precisionNotify(int);

    void moveToThread(QObject * obj);
private:
    QScopedPointer<TimeEvents> mTimeEvents;
    QScopedPointer<MassSpec> mMassSpec;
    QScopedPointer<TimeParams> mTimeParams;
    QScopedPointer<MassSpectrumsCollection> mMassSpecsColl;
    int mRealNumPrecision;
};

class SimpleThread : public QThread
{
public:
    void run();
};

#endif
