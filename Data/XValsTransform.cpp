#include "Data/TimeEvents.h"
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

double TimeScale::transform(double xVal) const
{
    const TimeParams * params = TimeParams::globalInstance();
    return xVal * params->mTimeFactor + params->mTimeOrigin;
}
