#include "LogSplinePoissonWeight.h"
#include "ParSplineCalc.h"
#include "Solvers.h"

#include <random>

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
        calc->logSplinePoissonWeights(yOut, yIn, *m_p);
        double TIC = std::accumulate(yOut.begin(), yOut.end(), 0.0);
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
                TIC = std::accumulate(yOut.begin(), yOut.end(), 0.0);
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
                TIC = std::accumulate(yOut.begin(), yOut.end(), 0.0);
            }
            a = *m_p / 10.; b = *m_p;
        }

        math::froot
        (
            [&](double x)->double
            {
                calc->logSplinePoissonWeights
                (
                    yOut,
                    yIn,
                    x
                );
                return sqDif(yOut, yIn) - std::accumulate(yOut.begin(), yOut.end(), 0.0);
            },
            a, b
        );
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
		VectorDouble tmpYIn(yIn);

		std::for_each(tmpYIn.begin(), tmpYIn.end(), [=](double& y) 
		{
			y = y / *m_noiseLevel;
		});

        ParSplineCalc::InstanceLocker calc
        (
            ParSplineCalc::lockInstance()
        );

        calc->logSplinePoissonWeights(yOut, tmpYIn, *m_p);

        double a, b;
        if(peakCount(yOut) > *m_peakCount)
        {
            while(peakCount(yOut) > *m_peakCount)
                calc->logSplinePoissonWeights(yOut, tmpYIn, *m_p *= 10.);
            a = *m_p / 10.;
            b = *m_p;
        }
        else
        {
            while(peakCount(yOut) <= *m_peakCount)
                calc->logSplinePoissonWeights(yOut, tmpYIn, *m_p /= 10.);
            a = *m_p;
            b = *m_p * 10.;
        }

        while((b - a) / (b + a) > std::numeric_limits<double>::epsilon())
        {
            calc->logSplinePoissonWeights(yOut, tmpYIn, *m_p = .5*(a + b));
            if(peakCount(yOut) > *m_peakCount) a = *m_p;
            else b = *m_p;
        }
        calc->logSplinePoissonWeights(yOut, tmpYIn, b);
        *m_p = b;
        std::for_each(yOut.begin(), yOut.end(), [=](double& y)
        {
            y *= *m_noiseLevel;
        });
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

LSFixNoiseValue::LSFixNoiseValue(const QVariantMap &pars)
    :
      Smoother (pars, paramsTemplate()),
      m_p(paramPtr<double>(SMOOTH_PARAM)),
      m_noise(paramPtr<double>(NOISE_LEVEL))
{

}

Smoother::Type LSFixNoiseValue::type() const
{
    return LogSplineFixNoiseValue;
}

void LSFixNoiseValue::run
(
    VectorDouble &yOut,
    const VectorDouble &yIn
)
{
    if(inputCheck(yOut, yIn))
    {
        ParSplineCalc::InstanceLocker calc
        (
            ParSplineCalc::lockInstance()
        );
        calc->logSplinePoissonWeights(yOut, yIn, *m_p);
        double s = std(yOut, yIn);
        double a, b;
        if(s > *m_noise)
        {
            while (s > *m_noise)
            {
                calc->logSplinePoissonWeights
                (
                    yOut,
                    yIn,
                    *m_p /= 10.
                );
                s = std(yOut, yIn);
            }
            a = *m_p; b = *m_p * 10.;
        }
        else
        {
            while(s < *m_noise)
            {
                calc->logSplinePoissonWeights
                (
                    yOut,
                    yIn,
                    *m_p *= 10.
                );
                s = std(yOut, yIn);
            }
            a = *m_p / 10.; b = *m_p;
        }

        *m_p = math::froot
        (
            [&](double x)->double
            {
                calc->logSplinePoissonWeights
                (
                    yOut,
                    yIn,
                    x
                );
                return std(yOut, yIn) - *m_noise;
            },
            a,
            b
        );

        m_maxPeakPos = maxPeakPos(yOut);

        m_maxPeakPosUncertainty = 0.0;

        const size_t nRuns = 10;
        VectorDouble yy(yOut.size()), yyOut(yOut.size());
        std::poisson_distribution<> dist;
        std::mt19937_64 gen;
        for(size_t i = 0; i < nRuns; ++i)
        {
            for(size_t j = 0; j < yOut.size(); ++j)
            {
                if(yOut[j] > 0.0)
                {
                    dist.param(std::poisson_distribution<>::param_type(yOut[j]));
                    yy[j] = dist(gen);
                }
                else
                {
                    yy[j] = 0.0;
                }
            }
            calc->logSplinePoissonWeights(yyOut, yy, *m_p);
            double d = maxPeakPos(yyOut) - m_maxPeakPos;
            m_maxPeakPosUncertainty += d*d;
        }
        m_maxPeakPosUncertainty = std::sqrt(m_maxPeakPosUncertainty/nRuns);
    }
}

QVariantMap LSFixNoiseValue::paramsTemplate() const
{
    QVariantMap params;
    params[SMOOTH_PARAM] = QVariant::fromValue<double>(1.0);
    params[NOISE_LEVEL] = QVariant::fromValue<double>(0.1);
    return params;
}

void LSFixNoiseValue::setParams(const QVariantMap &params)
{
    Smoother::setParams(params);
    m_p = paramPtr<double>(SMOOTH_PARAM);
    m_noise = paramPtr<double>(NOISE_LEVEL);
}

double LSFixNoiseValue::peakPosition() const
{
    return m_maxPeakPos;
}

double LSFixNoiseValue::peakPositionUncertainty() const
{
    return m_maxPeakPosUncertainty;
}

