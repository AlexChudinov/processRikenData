#ifndef ALGLIBSPLINE_H
#define ALGLIBSPLINE_H

#include "Smoother.h"

class AlglibSpline : public Smoother
{
public:
    AlglibSpline(const QVariantMap& pars);

    virtual Type type() const;

    virtual void run
    (
        VectorDouble& yOut,
        const VectorDouble& yIn
    );

    virtual QVariantMap paramsTemplate() const;
    virtual void setParams(const QVariantMap& params);
private:
    const int * mM; //Number of spline points
};

#endif // ALGLIBSPLINE_H
