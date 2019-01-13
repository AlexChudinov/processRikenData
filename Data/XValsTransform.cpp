#include "Data/TimeEvents.h"
#include "Base/BaseObject.h"
#include "XValsTransform.h"

XValsTransform::XValsTransform()
{

}

XValsTransform::~XValsTransform()
{

}

TimeScale::TimeScale()
    :
      XValsTransform()
{

}

XValsTransform::Type TimeScale::type() const
{
    return Time;
}

double TimeScale::transform(double xVal) const
{
    const TimeParams * params = MyInit::instance()->timeParams();
    return xVal * params->mTimeFactor + params->mTimeOrigin;
}

const QString &TimeScale::xUnits() const
{
    static QString timeUnits;
    const TimeParams * params = MyInit::instance()->timeParams();
    if(timeUnits.isEmpty())
    {
        timeUnits = "Time [" + params->mTimeUnits + "]";
    }
    return timeUnits;
}
