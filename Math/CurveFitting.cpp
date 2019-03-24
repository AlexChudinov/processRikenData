#include "CurveFitting.h"
#include "Math/alglib/optimization.h"
#include "../Base/ThreadPool.h"
#include <cassert>

const QStringList &CurveFitting::implementations()
{
    static QStringList g_impl{"Asymmetric Gaussian"};
    return g_impl;
}

CurveFitting::Ptr CurveFitting::create(const QString &name, const DoubleVector &x, const DoubleVector &y)
{
    if(name == "Asymmetric Gaussian")
        return Ptr(new AsymmetricGaussian(x, y));
    else return Ptr();
}

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
    DoubleVector res
    {
        mParams->mW,
        mParams->mTc,
        mParams->mDTL,
        mParams->mDTR
    };
    cv::Ptr<cv::DownhillSolver> solver(cv::DownhillSolver::create());
    solver->setInitStep(res);
    solver->setFunction(cv::Ptr<Function>(new Function({this, x, y})));
    solver->minimize(res);
}

void AsymmetricGaussian::values
(
    const CurveFitting::DoubleVector &x,
    CurveFitting::DoubleVector &y
) const
{
    y.assign(x.size(), 0.0);
    ThreadPool::parFor(x.size(),[&](size_t i)
    {
        y[i] = value(x[i]);
    });
}

QString AsymmetricGaussian::eqn() const
{
    return QString("Asymmetric Gaussian");
}

CurveFitting::ParamsList AsymmetricGaussian::params() const
{
    ParamsList params
    {
        {"A", mParams->mA},
        {"dtL", mParams->mDTL},
        {"dtR", mParams->mDTR},
        {"tc", mParams->mTc},
        {"w", mParams->mW}
    };
    return params;
}

void AsymmetricGaussian::setParams(const ParamsList& params)
{
    bool ok = true;
    QVariantMap::ConstIterator it = params.find("A");
    if(it != params.end())
        mParams->mA = it.value().toDouble(&ok);
    if((it = params.find("dtL")) != params.end())
        mParams->mDTL = it.value().toDouble(&ok);
    if((it = params.find("dtR")) != params.end())
        mParams->mDTR = it.value().toDouble(&ok);
    if((it = params.find("tc")) != params.end())
        mParams->mTc = it.value().toDouble(&ok);
    if((it = params.find("w")) != params.end())
        mParams->mW = it.value().toDouble(&ok);
    Q_ASSERT(ok);
}

CurveFitting::ParamsList AsymmetricGaussian::errors() const
{
    ParamsList errors
    {
        {"A", mParams->mA},
        {"dtL", mErrors->mDTL},
        {"dtR", mErrors->mDTR},
        {"tc", mErrors->mTc},
        {"w", mErrors->mW}
    };
    return errors;
}

QTextStream &AsymmetricGaussian::operator<<(QTextStream &out) const
{
    out << "Fitted with " << eqn() << "\n";
    QVariantMap pars = params();
    QVariantMap errs = errors();
    bool ok = true;
    for
    (
        QVariantMap::ConstIterator itPars = pars.begin(), itErrs = errs.begin();
        itPars != pars.end();
        ++itPars, ++itErrs
    )
    {
        out << itPars.key() << " = " << itPars.value().toDouble(&ok)
            << "+/-" << itErrs.value().toDouble(&ok) << "\n";
    }
    Q_ASSERT(ok);
    return out;
}

void AsymmetricGaussian::init(const CurveFitting::DoubleVector &x, const CurveFitting::DoubleVector &y)
{
    mParams.reset(new Parameters);
    mErrors.reset(new Errors);
    *mParams = {1., 0., 0., 0., 0.};
    *mErrors = {0., 0., 0., 0., 0.};
    double norm = 0.0, xx = 0.0, xx2 = 0.0;
    for(size_t i = 0; i < x.size(); ++i)
    {
        norm += y[i];
        xx += y[i]*x[i];
        xx2 += y[i]*x[i]*x[i];
    }
    xx /= norm;
    mParams->mTc = xx;
    mParams->mW = xx2 / norm - xx * xx;
    mParams->mDTL = mParams->mW / 2;
    mParams->mDTR = mParams->mDTL;
    curveScaling(x, y);
}

void AsymmetricGaussian::curveScaling(const CurveFitting::DoubleVector &x, const CurveFitting::DoubleVector &y)
{
    DoubleVector ty;
    mParams->mA = 1.0;
    values(x, ty);
    double A = 0., norm = 0.;
    for(size_t i = 0; i < x.size(); ++i)
    {
        A += y[i] * ty[i];
        norm += ty[i] * ty[i];
    }
    mParams->mA = A / norm;
}

void AsymmetricGaussian::minFun(const alglib::real_1d_array &pars, alglib::real_1d_array &s, void * ptr)
{
    OptimizationData * d = static_cast<OptimizationData*>(ptr);
    AsymmetricGaussian * obj = std::get<0>(*d);
    const DoubleVector& x = std::get<1>(*d);
    const DoubleVector& y = std::get<2>(*d);

    obj->mParams->mW = pars[0];
    obj->mParams->mTc = pars[1];
    obj->mParams->mDTL = pars[2];
    obj->mParams->mDTR = pars[3];

    static DoubleVector ty;

    obj->values(x, ty);

    for(size_t i = 0; i < ty.size(); ++i) s[0] += (ty[i] - y[i]) * (ty[i] - y[i]);
    double dd  = s[0];
    s[1] = 0.; s[2] = 0.; s[3] = 0.;
}

int AsymmetricGaussian::Function::getDims() const
{
    return 4;
}

double AsymmetricGaussian::Function::calc(const double *x) const
{
    AsymmetricGaussian * obj = std::get<0>(mData);
    const DoubleVector& xx = std::get<1>(mData);
    const DoubleVector& yy = std::get<2>(mData);
    obj->mParams->mW = x[0];
    obj->mParams->mTc = x[1];
    obj->mParams->mDTL = x[2];
    obj->mParams->mDTR = x[3];
    obj->curveScaling(xx, yy);
    QMutex mut;
    double s = 0.;
    ThreadPool::parFor(xx.size(), [&](size_t i)
    {
        double ss = obj->value(xx[i]) - yy[i];
        QMutexLocker lock(&mut);
        s += (ss * ss);
    });
    return s;
}
