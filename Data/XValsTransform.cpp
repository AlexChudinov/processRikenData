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

QString TimeScale::xUnits() const
{
    const TimeParams * params = MyInit::instance()->timeParams();
    return "Time [" + params->mTimeUnits + "]";
}
