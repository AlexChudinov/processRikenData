#include "LogSplinePoissonWeight.h"
#include "ParSplineCalc.h"

LogSplinePoissonWeight::LogSplinePoissonWeight
(
    const QVariantMap &pars
)
    :
      Smoother(pars),
      m_p
      (
        *reinterpret_cast<const double*>(params().begin().value().data())
      )
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

        calc->logSplinePoissonWeights(yOut, yIn, m_p);
    }
}

LogSplinePoissonWeightPoissonNoise::LogSplinePoissonWeightPoissonNoise
(
    const QVariantMap &
)
    :
      Smoother(QVariantMap({{ "Smoothness parameter", QVariant(1.0) }})),
      m_p(*reinterpret_cast<double*>(params().begin().value().data()))
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
        calc->logSplinePoissonWeights(yOut, yIn, m_p);
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
                    m_p /= 10.
                );
                s = sqDif(yOut, yIn);
            }
            a = m_p; b = m_p * 10.;
        }
        else
        {
            while(s < TIC)
            {
                calc->logSplinePoissonWeights
                (
                    yOut,
                    yIn,
                    m_p *= 10.
                );
                s = sqDif(yOut, yIn);
            }
            a = m_p / 10.; b = m_p;
        }

        while(std::abs(a - b) > 1.0)
        {
            calc->logSplinePoissonWeights
            (
                yOut,
                yIn,
                m_p = .5 * (a + b)
            );
            s = sqDif(yOut, yIn);
            if(s < TIC) a = m_p;
            else if (s > TIC) b = m_p;
            else break;
        }
    }
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
