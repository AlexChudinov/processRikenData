#include "LogSplinePoissonWeight.h"

Smoother::Smoother(const QVariantMap &pars, QVariantMap &&parsTemp)
{
    //Checking of inputed table
    if(parsTemp.size() == pars.size())
    {
        bool ok = true;
        QVariantMap::ConstIterator
                it1 = pars.cbegin(),
                it2 = parsTemp.cbegin();
        for(; it1 != pars.cend(); ++it1, ++it2)
        {
            ok &= it1.key() == it2.key();
            ok &= it1.value().type() == it2.value().type();
        }
        if(ok) m_params = pars;
        else m_params = std::move(parsTemp);
    }
    else m_params = std::move(parsTemp);
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
        return Pointer(new LogSplinePoissonWeightPoissonNoise);
    case LogSplinePoissonWeightOnePeakType:
        return Pointer(new LogSplinePoissonWeightOnePeak(pars));
    }
    return Pointer();
}

void Smoother::setParams(const QVariantMap &params)
{
    //Checking of inputed table
    if(m_params.size() == params.size())
    {
        bool ok = true;
        QVariantMap::ConstIterator
                it1 = m_params.cbegin(),
                it2 = params.cbegin();
        for(; it1 != m_params.cend(); ++it1, ++it2)
        {
            ok &= it1.key() == it2.key();
            ok &= it1.value().type() == it2.value().type();
        }
        if(ok) m_params = params;
    }
}
