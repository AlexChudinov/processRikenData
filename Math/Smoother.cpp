#include "LogSplinePoissonWeight.h"

QMap<QString, Smoother::Type> Smoother::s_registry
{
    {"LogSplinePoissonWeightType", Smoother::LogSplinePoissonWeightType},
    {"LogSplinePoissonWeightPoissonNoiseType", Smoother::LogSplinePoissonWeightPoissonNoiseType},
    {"LogSplinePoissonWeightOnePeakType", Smoother::LogSplinePoissonWeightOnePeakType},
    {"LogSplineFixNoiseValue", Smoother::LogSplineFixNoiseValue}
};

QStringList Smoother::s_typeStrings
{
    "LogSplinePoissonWeightType",
    "LogSplinePoissonWeightPoissonNoiseType",
    "LogSplinePoissonWeightOnePeakType",
    "LogSplineFixNoiseValue"
};

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
    case LogSplineFixNoiseValue:
        return Pointer(new LSFixNoiseValue(pars));
    }
    return Pointer();
}

Smoother::Pointer Smoother::create(const QString &typeName, const QVariantMap &pars)
{
    QMap<QString, Type>::ConstIterator it = s_registry.find(typeName);
    if (it != s_registry.end()) return create(it.value(), pars);
    else return Pointer();
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

double Smoother::maxPeakPos(const VectorDouble &y)
{
    VectorDouble::const_iterator it = std::max_element(y.cbegin(), y.cend());
    const ptrdiff_t n = std::distance(y.cbegin(), it);
    double b = y[n+1] - y[n-1];
    double a = y[n+1] - 2*y[n] + y[n-1];
    return static_cast<double>(n) - b / 2 / a;
}

double Smoother::sqDif
(
    const VectorDouble &y1,
    const VectorDouble &y2
)
{
    double res = 0;
    for(size_t i = 0; i < y1.size(); ++i)
    {
        res += (y1[i] - y2[i]) * (y1[i] - y2[i]);
    }
    return res;
}

double Smoother::std(const Smoother::VectorDouble &y1, const Smoother::VectorDouble &y2)
{
    return sqDif(y1, y2) / y1.size();
}
