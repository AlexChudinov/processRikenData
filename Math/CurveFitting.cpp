#include <QMessageBox>
#include <Eigen/Dense>
#include <array>
#include "CurveFitting.h"
#include "../Base/ThreadPool.h"
#include "../QMapPropsDialog.h"

using namespace Eigen;
using namespace std;

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
    Q_ASSERT(x.size() == y.size());
}

CurveFitting::~CurveFitting()
{

}

AsymmetricGaussian::AsymmetricGaussian
(
    const DoubleVector &x,
    const DoubleVector &y,
    const AsymmetricGaussian &other
)
    :
      CurveFitting (x, y),
      mParams(new Parameters(*other.mParams)),
      mErrors(new Errors(*other.mErrors)),
      mProps(new Properties(*other.mProps))
{
    run(x, y);
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
    QMapPropsDialog dialog;
    dialog.setProps(properties());
    dialog.exec();
    setProperties(dialog.props());

    if(run(x, y) == 0.0)
    {
        QMessageBox::warning
        (
            Q_NULLPTR,
            "Fitting message",
            "Erroneous result. Please, try to do another fitting."
        );
    }
    else
    {
        estimateErrors(x);
        QString fitting;
        QTextStream stream(&fitting);
        print(stream);
        stream << "sig = " << residuals(x, y) << "\n";
        stream.flush();
        QMessageBox msg
        (
            QMessageBox::Information,
            "Curve fitting",
            *stream.string()
        );
        msg.exec();
    }
}

void AsymmetricGaussian::values
(
    const CurveFitting::DoubleVector &x,
    CurveFitting::DoubleVector &y
) const
{
    y.resize(x.size());
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
        {"A", mErrors->mA},
        {"dtL", mErrors->mDTL},
        {"dtR", mErrors->mDTR},
        {"tc", mErrors->mTc},
        {"w", mErrors->mW}
    };
    return errors;
}

CurveFitting::ParamsList AsymmetricGaussian::properties() const
{
    ParamsList props
    {
        {"Init. step", mProps->mStep},
        {"Rel. tolerance", mProps->mRelTol},
        {"Iter. num", mProps->mIterNum}
    };
    return props;
}

void AsymmetricGaussian::setProperties
(
    const CurveFitting::ParamsList &props
)
{
    bool ok = true;
    ParamsList::ConstIterator it = props.find("Init. step");
    if(it != props.end())
        mProps->mStep = it.value().toDouble(&ok);
    if((it = props.find("Rel. tolerance")) != props.end())
        mProps->mRelTol = it.value().toDouble(&ok);
    if((it = props.find("Iter. num")) != props.end())
        mProps->mIterNum = it.value().toDouble(&ok);
    Q_ASSERT(ok);
}

void AsymmetricGaussian::print(QTextStream &out) const
{
    out << "Fitted with " << eqn() << "\n";
    QVariantMap pars = params();
    QVariantMap errs = errors();
    out.setRealNumberPrecision(10);
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
}

double AsymmetricGaussian::peakPosition() const
{
    return mParams->mTc;
}

double AsymmetricGaussian::peakPositionUncertainty() const
{
    return mErrors->mTc;
}

double AsymmetricGaussian::run
(
    const DoubleVector &x,
    const DoubleVector &y
)
{
    double A;
    size_t iterNum = 0;
    do
    {
        A = mParams->mA;
        cv::Mat_<double> res(4, 1);
        res << mParams->mW, mParams->mTc,
            mParams->mDTL, mParams->mDTR;
        cv::Ptr<cv::DownhillSolver> solver
        (
            cv::DownhillSolver::create
            (
                cv::Ptr<Function>(new Function(this, x, y))
            )
        );
        solver->setInitStep(0.1 * res);
        solver->setTermCriteria(cv::TermCriteria(3, 10000, A * mProps->mRelTol));
        solver->minimize(res);
        mParams->mW = res(0);
        mParams->mTc = res(1);
        mParams->mDTL = res(2);
        mParams->mDTR = res(3);
        curveScaling(x, y);
    }
    while
    (
        A != 0.0
        && std::fabs(A - mParams->mA)/A > mProps->mRelTol
        && (++iterNum < mProps->mIterNum)
    );
    return A;
}

void AsymmetricGaussian::init(const CurveFitting::DoubleVector &x, const CurveFitting::DoubleVector &y)
{
    mParams.reset(new Parameters);
    mErrors.reset(new Errors);
    mProps.reset(new Properties);
    *mParams = {1., 0., 0., 0., 0.};
    *mErrors = {0., 0., 0., 0., 0.};
    *mProps = {0.25, 1e-6, 10};
    double norm = 0.0, xx = 0.0, xx2 = 0.0;
    for(size_t i = 0; i < x.size(); ++i)
    {
        norm += y[i];
        xx += y[i]*x[i];
        xx2 += y[i]*x[i]*x[i];
    }
    xx /= norm;
    mParams->mTc = xx;
    mParams->mW = ::sqrt(xx2 / norm - xx * xx);
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
    mParams->mA = norm != 0.0 ? A / norm : A;
}

void AsymmetricGaussian::estimateErrors
(
    const CurveFitting::DoubleVector &x
)
{
    const size_t nRuns = 10;
    DoubleVector y0;
    values(x, y0);
    std::poisson_distribution<> dist;
    std::mt19937_64 gen;
    double sig = 0.0;
    for(size_t i = 0; i < nRuns; ++i)
    {
        DoubleVector yy(y0.size());
        for(size_t j = 0; j < yy.size(); ++j)
        {
            dist.param(std::poisson_distribution<>::param_type(y0[j]));
            yy[j] = dist(gen);
        }
        AsymmetricGaussian gaus(x, yy, *this);
        double d = mParams->mTc - gaus.mParams->mTc;
        sig += d * d;
    }
    sig /= nRuns;
    mErrors->mTc = std::sqrt(sig);
}

double AsymmetricGaussian::dfdA(double x) const
{
    const double dx = (x - mParams->mTc) / mParams->mW;
    const double dxL = mParams->mDTL / mParams->mW;
    const double dxR = mParams->mDTR / mParams->mW;
    if(dx < - dxL)
        return std::exp(dxL * dx + .5 * dxL * dxL);
    else if(dx > dxR)
        return std::exp(.5 * dxR * dxR - dxR * dx);
    else
        return std::exp(-.5 * dx * dx);
}

double AsymmetricGaussian::dfdw(double x) const
{
    const double df = value(x);
    const double dx = (x - mParams->mTc) / mParams->mW;
    const double dxL = mParams->mDTL / mParams->mW;
    const double dxR = mParams->mDTR / mParams->mW;
    const double factor = - 2. / mParams->mW;
    if(dx < -dxL)
        return factor * (dxL * dx + .5 * dxL * dxL) * df;
    else if (dx > dxR)
        return factor * (.5 * dxR * dxR - dxR * dx) * df;
    else
        return -.5 * factor * dx * dx * df;
}

double AsymmetricGaussian::dfdtL(double x) const
{
    const double f = value(x);
    const double dx = (x - mParams->mTc) / mParams->mW;
    const double dxL = mParams->mDTL / mParams->mW;
    if(dx >= -dxL) return 0.0;
    else return (dx + dxL) * f / mParams->mW;
}

double AsymmetricGaussian::dfdtR(double x) const
{
    const double f = value(x);
    const double dx = (x - mParams->mTc) / mParams->mW;
    const double dxR = mParams->mDTR / mParams->mW;
    if(dx <= dxR) return 0.0;
    else return (dxR - dx) * f / mParams->mW;
}

double AsymmetricGaussian::dfdtc(double x) const
{
    const double f = value(x);
    const double dx = (x - mParams->mTc) / mParams->mW;
    const double dxL = mParams->mDTL / mParams->mW;
    const double dxR = mParams->mDTR / mParams->mW;
    if(dx < -dxL)
        return - dxL * f / mParams->mW;
    else if (dx > dxR)
        return  dxR * f / mParams->mW;
    else
        return dx * f / mParams->mW;
}

double AsymmetricGaussian::residuals
(
    const CurveFitting::DoubleVector &x,
    const CurveFitting::DoubleVector &y
) const
{
    double s = 0.;
    CurveFitting::DoubleVector yy;
    values(x, yy);
    for(size_t i = 0; i < x.size(); ++i)
    {
        s += (y[i] - yy[i]) * (y[i] - yy[i]);
    }
    return s / (y.size() - 5);
}

int AsymmetricGaussian::Function::getDims() const
{
    return 4;
}

double AsymmetricGaussian::Function::calc(const double *x) const
{
    mObj->mParams->mW = x[0];
    mObj->mParams->mTc = x[1];
    mObj->mParams->mDTL = x[2];
    mObj->mParams->mDTR = x[3];
    mObj->curveScaling(m_x, m_y);
    double s = 0.;
    for(size_t i = 0; i < m_x.size(); ++i)
    {
        double ss = mObj->value(m_x[i]) - m_y[i];
        s += ss*ss;
    }
    return s;
}

void AsymmetricGaussian::Function::getGradient(const double *x, double *y)
{
    mObj->mParams->mW = x[0];
    mObj->mParams->mTc = x[1];
    mObj->mParams->mDTL = x[2];
    mObj->mParams->mDTR = x[3];
    mObj->curveScaling(m_x, m_y);
    DoubleVector yy;
    mObj->values(m_x, yy);
    y[0] = 0.0;
    y[1] = 0.0;
    y[2] = 0.0;
    y[3] = 0.0;
    for(size_t i = 0; i < m_x.size(); ++i)
    {
        double d = yy[i] - m_y[i];
        y[0] += d * mObj->dfdw(m_x[i]);
        y[1] += d * mObj->dfdtc(m_x[i]);
        y[2] += d * mObj->dfdtL(m_x[i]);
        y[3] += d * mObj->dfdtR(m_x[i]);
    }
    y[0] *= 2.0;
    y[1] *= 2.0;
    y[2] *= 2.0;
    y[3] *= 2.0;
}
