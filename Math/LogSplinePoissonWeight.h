#ifndef LOGSPLINEPOISSONWEIGHT_H
#define LOGSPLINEPOISSONWEIGHT_H

#include "Smoother.h"
#include "peakparams.h"

/**
 * @brief The LogSplinePoissonWeight class calculates log spline
 * with poisson weights. Spline parameter p should be supplied.
 */
class LogSplinePoissonWeight : public Smoother
{
    const double* m_p;
public:

    LogSplinePoissonWeight(const QVariantMap& pars);

    Type type() const;

    void run(VectorDouble& yOut, const VectorDouble& yIn);

    QVariantMap paramsTemplate() const;

    void setParams(const QVariantMap &params);
};

/**
 * @brief The LogSplinePoissonWeightPoissonNoise class paramless
 * log spline smoothing. With poisson noise assumption
 */
class LogSplinePoissonWeightPoissonNoise : public Smoother
{
    double* m_p;
public:

    LogSplinePoissonWeightPoissonNoise();

    Type type() const;

    void run(VectorDouble &yOut, const VectorDouble &yIn);

    QVariantMap paramsTemplate() const;

    void setParams(const QVariantMap &params);
};

/**
 * @brief The LogSplinePoissonWeightOnePeak class regulates smoothness
 * to fit one peak
 */
class LogSplinePoissonWeightOnePeak : public Smoother
{
    double * m_p;
    const int * m_peakCount;
    const double * m_noiseLevel;
public:

    LogSplinePoissonWeightOnePeak(const QVariantMap& pars);

    Type type() const;

    void run(VectorDouble &yOut, const VectorDouble &yIn);

    QVariantMap paramsTemplate() const;

    void setParams(const QVariantMap &params);
private:
    //Checks that there is only one peak
    inline int peakCount(const VectorDouble& y)
    {
        int cnt = 0;
        for(size_t i = 1; i < y.size() - 1; ++i)
            if(y[i-1] < y[i] && y[i] > y[i+1]
                    && y[i] > 1.0) cnt++;
        return cnt;
    }
};

class LSFixNoiseValue : public Smoother, public PeakParams
{
    double * m_p;
    double * m_noise;
    double m_maxPeakPos;
    double m_maxPeakPosUncertainty;
public:
    LSFixNoiseValue(const QVariantMap& pars);

    Type type() const;

    void run(VectorDouble& yOut, const VectorDouble& yIn);

    QVariantMap paramsTemplate() const;

    void setParams(const QVariantMap& params);

    double peakPosition() const;

    double peakPositionUncertainty() const;
};

#endif // LOGSPLINEPOISSONWEIGHT_H
