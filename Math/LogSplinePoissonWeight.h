#ifndef LOGSPLINEPOISSONWEIGHT_H
#define LOGSPLINEPOISSONWEIGHT_H

#include "Smoother.h"

/**
 * @brief The LogSplinePoissonWeight class calculates log spline
 * with poisson weights. Spline parameter p should be supplied.
 */
class LogSplinePoissonWeight : public Smoother
{
    const double& m_p;
public:

    LogSplinePoissonWeight(const QVariantMap& pars);

    Type type() const;

    void run(VectorDouble& yOut, const VectorDouble& yIn);
};

/**
 * @brief The LogSplinePoissonWeightPoissonNoise class paramless
 * log spline smoothing. With poisson noise assumption
 */
class LogSplinePoissonWeightPoissonNoise : public Smoother
{
    double& m_p;
public:

    LogSplinePoissonWeightPoissonNoise(const QVariantMap&);

    Type type() const;

    void run(VectorDouble &yOut, const VectorDouble &yIn);
private:
    /**
     * @brief sqDif square deviances between two arrays
     * @param y1
     * @param y2
     * @return
     */
    static double sqDif
    (
        const VectorDouble& y1,
        const VectorDouble& y2
    );
};

#endif // LOGSPLINEPOISSONWEIGHT_H
