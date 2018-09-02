#include "LogSplinePoissonWeight.h"

Smoother::Smoother(const QVariantMap &pars)
    :
      m_params(pars)
{

}

Smoother::~Smoother()
{

}

Smoother::Pointer Smoother::create
(
    Smoother::Type type,
    const QVariantMap &pars
)
{
    switch(type)
    {
    case LogSplinePoissonWeightType:
        return Pointer(new LogSplinePoissonWeight(pars));
    case LogSplinePoissonWeightPoissonNoiseType:
        return Pointer(new LogSplinePoissonWeightPoissonNoise(pars));
    }
    return Pointer();
}
