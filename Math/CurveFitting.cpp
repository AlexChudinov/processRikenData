#include <QMessageBox>
#include <Eigen/Dense>
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
                cv::Ptr<Function>(new Function(this, x, y)),
                res
            )
        );
        solver->setInitStep(mProps->mStep * res);
        solver->minimize(res);
        solver->setTermCriteria(cv::TermCriteria(3, 10000, A * mProps->mRelTol));
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
    if(A == 0.0)
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
        estimateErrors(x, y);
        QString fitting;
        QTextStream stream(&fitting);
        *this >> stream;
        stream << "sig = " << residuals(x, y) << "\n";
        stream.flush();
        QMessageBox::warning
        (
            Q_NULLPTR,
            "Fitting result",
            fitting
        );
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

QTextStream &AsymmetricGaussian::operator>>(QTextStream &out) const
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
    mProps.reset(new Properties);
    *mParams = {1., 0., 0., 0., 0.};
    *mErrors = {0., 0., 0., 0., 0.};
    *mProps = {0.1, 1e-6, 10000};
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
    const CurveFitting::DoubleVector &x,
    const CurveFitting::DoubleVector &y
)
{
    const size_t n = x.size();
    DoubleVector
            dfdA_vals(n), dfdw_vals(n),
            dfdtL_vals(n), dfdtR_vals(n),
            dfdtc_vals(n);
    MatrixXd M(n, 5);
    VectorXd V(x.size());
    V.fill(1.0);
    for(size_t i = 0; i < x.size(); ++i)
    {
        M(i, 0) = dfdA(x[i]);
        M(i, 1) = dfdw(x[i]);
        M(i, 2) = dfdtL(x[i]);
        M(i, 3) = dfdtR(x[i]);
        M(i, 4) = dfdtc(x[i]);
    }
    MatrixXd TM = M.transpose();
    double sig = residuals(x, y);
    VectorXd err = sig * (TM * M).ldlt().solve(TM*V);
    double f = x.size() - 5;
    mErrors->mA = std::sqrt(std::fabs(err(0)/f));
    mErrors->mW = std::sqrt(std::fabs(err(1)/f));
    mErrors->mDTL = std::sqrt(std::fabs(err(2)/f));
    mErrors->mDTR = std::sqrt(std::fabs(err(3)/f));
    mErrors->mTc = std::sqrt(std::fabs(err(4)/f));
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
    QMutex mut;
    double s = 0.;
    ThreadPool::parFor(m_x.size(), [&](size_t i)
    {
        double ss = mObj->value(m_x[i]) - m_y[i];
        QMutexLocker lock(&mut);
        s += (ss * ss);
    });
    return s;
}
