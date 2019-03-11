#ifndef XVALSTRANSFORM_H
#define XVALSTRANSFORM_H

class QString;

/**
 * @brief The XValsTransform class transform x vals induces to time or mass units
 */

class XValsTransform
{
public:
    XValsTransform();
    virtual ~XValsTransform();

    enum Type { Time, Mass };

    virtual Type type() const = 0;

    /**
     * @brief transform transforms signal from ms induces to x-values
     * @param xVal
     * @return
     */
    virtual double transform(double xVal) const = 0;

    /**
     * @brief invTransform transforms signal from x-values to ms induces
     * @param xVal
     * @return
     */
    virtual double invTransform(double xVal) const = 0;

    virtual QString xUnits() const = 0;
};

class TimeScale : public XValsTransform
{
public:
    TimeScale();

    virtual Type type() const;

    virtual double transform(double xVal) const;

    virtual double invTransform(double xVal) const;

    virtual QString xUnits() const;
};

#endif // XVALSTRANSFORM_H
