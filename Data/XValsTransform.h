#ifndef XVALSTRANSFORM_H
#define XVALSTRANSFORM_H

/**
 * @brief The XValsTransform class transform x vals induces to time or mass units
 */

class XValsTransform
{
public:
    XValsTransform();
    virtual ~XValsTransform();

    virtual double transform(double xVal) const = 0;
};

class TimeScale : public XValsTransform
{
public:
    TimeScale();

    virtual double transform(double xVal) const;
};

#endif // XVALSTRANSFORM_H
