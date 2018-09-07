#include "LogSplinePoissonWeight.h"
#include "ParSplineCalc.h"

LogSplinePoissonWeight::LogSplinePoissonWeight
(
    const QVariantMap &pars
)
    :
      Smoother(pars, paramsTemplate()),
      m_p(paramPtr<double>(SMOOTH_PARAM))
{}

Smoother::Type LogSplinePoissonWeight::type() const
{
    return LogSplinePoissonWeightType;
}

void LogSplinePoissonWeight::run
(
    Smoother::VectorDouble &yOut,
    const Smoother::VectorDouble &yIn
)
{
    if(inputCheck(yOut, yIn))
    {
        ParSplineCalc::InstanceLocker calc
        (
            ParSplineCalc::lockInstance()
        );

        calc->logSplinePoissonWeights(yOut, yIn, *m_p);
    }
}

QVariantMap LogSplinePoissonWeight::paramsTemplate() const
{
    return QVariantMap({{SMOOTH_PARAM, QVariant(1.0)}});

}

void LogSplinePoissonWeight::setParams(const QVariantMap &params)
{
    this->Smoother::setParams(params);
    m_p = paramPtr<double>(SMOOTH_PARAM);
}

LogSplinePoissonWeightPoissonNoise::LogSplinePoissonWeightPoissonNoise()
    :
      Smoother(QVariantMap(), paramsTemplate()),
      m_p(paramPtr<double>(SMOOTH_PARAM))
{
}

Smoother::Type LogSplinePoissonWeightPoissonNoise::type() const
{
    return LogSplinePoissonWeightPoissonNoiseType;
}

void LogSplinePoissonWeightPoissonNoise::run
(
    Smoother::VectorDouble &yOut,
    const Smoother::VectorDouble &yIn
)
{
    if(inputCheck(yOut, yIn))
    {
        ParSplineCalc::InstanceLocker calc
        (
            ParSplineCalc::lockInstance()
        );
        double TIC = std::accumulate(yIn.begin(), yIn.end(), 0.0);
        calc->logSplinePoissonWeights(yOut, yIn, *m_p);
        double s = sqDif(yOut, yIn);

        double a, b;
        if(s > TIC)
        {
            while (s > TIC)
            {
                calc->logSplinePoissonWeights
                (
                    yOut,
                    yIn,
                    *m_p /= 10.
                );
                s = sqDif(yOut, yIn);
            }
            a = *m_p; b = *m_p * 10.;
        }
        else
        {
            while(s < TIC)
            {
                calc->logSplinePoissonWeights
                (
                    yOut,
                    yIn,
                    *m_p *= 10.
                );
                s = sqDif(yOut, yIn);
            }
            a = *m_p / 10.; b = *m_p;
        }

        while(std::abs(a - b) > 1.0)
        {
            calc->logSplinePoissonWeights
            (
                yOut,
                yIn,
                *m_p = .5 * (a + b)
            );
            s = sqDif(yOut, yIn);
            if(s < TIC) a = *m_p;
            else if (s > TIC) b = *m_p;
            else break;
        }
    }
}

QVariantMap LogSplinePoissonWeightPoissonNoise::paramsTemplate() const
{
    return LogSplinePoissonWeight(QVariantMap()).paramsTemplate();
}

void LogSplinePoissonWeightPoissonNoise::setParams(const QVariantMap &params)
{
    this->Smoother::setParams(params);
    m_p = paramPtr<double>(SMOOTH_PARAM);
}

double LogSplinePoissonWeightPoissonNoise::sqDif
(
    const Smoother::VectorDouble &y1,
    const Smoother::VectorDouble &y2
)
{
    double res = 0;
    for(size_t i = 0; i < y1.size(); ++i)
    {
        res += (y1[i] - y2[i]) * (y1[i] - y2[i]);
    }
    return res;
}


LogSplinePoissonWeightOnePeak::LogSplinePoissonWeightOnePeak
(
    const QVariantMap &pars
)
:
  Smoother(pars, paramsTemplate()),
  m_p(paramPtr<double>(SMOOTH_PARAM)),
  m_peakCount(paramPtr<int>(PEAK_COUNT)),
  m_noiseLevel(paramPtr<double>(NOISE_LEVEL))
{
}

Smoother::Type LogSplinePoissonWeightOnePeak::type() const
{
    return LogSplinePoissonWeightOnePeakType;
}

void LogSplinePoissonWeightOnePeak::run
(
    Smoother::VectorDouble &yOut,
    const Smoother::VectorDouble &yIn
)
{
    if(inputCheck(yOut, yIn) && *m_peakCount != 0)
    {
        ParSplineCalc::InstanceLocker calc
        (
            ParSplineCalc::lockInstance()
        );

        calc->logSplinePoissonWeights(yOut, yIn, *m_p);

        double a, b;
        if(peakCount(yOut) > *m_peakCount)
        {
            while(peakCount(yOut) > *m_peakCount)
                calc->logSplinePoissonWeights(yOut, yIn, *m_p *= 10.);
            a = *m_p / 10., b = *m_p;
        }
        else
        {
            while(peakCount(yOut) <= *m_peakCount)
                calc->logSplinePoissonWeights(yOut, yIn, *m_p /= 10.);
            a = *m_p, b = *m_p * 10.;
        }

        while((b - a) / (b + a) > std::numeric_limits<double>::epsilon())
        {
            calc->logSplinePoissonWeights(yOut, yIn, *m_p = .5*(a + b));
            if(peakCount(yOut) > *m_peakCount) a = *m_p;
            else b = *m_p;
        }
        calc->logSplinePoissonWeights(yOut, yIn, b);
        *m_p = b;
    }
}

QVariantMap LogSplinePoissonWeightOnePeak::paramsTemplate() const
{
    QVariantMap res = LogSplinePoissonWeight(QVariantMap()).paramsTemplate();
    res[PEAK_COUNT] = QVariant::fromValue<int>(1ul);
    res[NOISE_LEVEL] = QVariant::fromValue<double>(1.0);
    return res;
}

void LogSplinePoissonWeightOnePeak::setParams(const QVariantMap &params)
{
    this->Smoother::setParams(params);
    m_p = paramPtr<double>(SMOOTH_PARAM);
    m_peakCount = paramPtr<int>(PEAK_COUNT);
    m_noiseLevel = paramPtr<double>(NOISE_LEVEL);
    if(*m_noiseLevel == 0.0)
    {
        *const_cast<double*>(m_noiseLevel) = 1.0;
    }
}
