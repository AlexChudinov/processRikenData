#include "CurveFitting.h"
#include "Math/alglib/optimization.h"
#include <QtConcurrent>
#include <cassert>

CurveFitting::CurveFitting(const DoubleVector &x, const DoubleVector &y)
{
    assert(x.size() == y.size());
}

CurveFitting::~CurveFitting()
{

}

AsymmetricGaussian::AsymmetricGaussian
(
    const CurveFitting::DoubleVector &x,
    const CurveFitting::DoubleVector &y
)
    :
      CurveFitting (x, y)
{
    init(x, y);

}

void AsymmetricGaussian::values
(
    const CurveFitting::DoubleVector &x,
    CurveFitting::DoubleVector &y
) const
{
    y.assign(x.size(), 0.0);
    QtConcurrent::map(y, [&](DoubleVector::reference yy)->void
    {
        const size_t i = static_cast<size_t>(&yy - y.data());
        yy = value(x[i]);
    }).waitForFinished();
}

QString AsymmetricGaussian::eqn() const
{
    return QString("Asymmetric Gaussian");
}

QVariantMap AsymmetricGaussian::params() const
{
    ParamsList params
    {
        {"dtL", mParams->mDTL},
        {"dtR", mParams->mDTR},
        {"tc", mParams->mTc},
        {"w", mParams->mW}
    };
    return params;
}

void AsymmetricGaussian::setParams(const QVariantMap& params)
{
    bool ok = true;
    QVariantMap::ConstIterator it = params.find("dtL");
    if(it != params.end())
        mParams->mDTL = it.value().toDouble(&ok);
    if((it = params.find("dtR")) != params.end())
        mParams->mDTR = it.value().toDouble(&ok);
    if((it = params.find("tc")) != params.end())
        mParams->mTc = it.value().toDouble(&ok);
    if((it = params.find("w")) != params.end())
        mParams->mW = it.value().toDouble(&ok);
    Q_ASSERT(ok);
}
