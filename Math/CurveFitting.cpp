#include <QMessageBox>
#include <Eigen/Dense>
#include <array>
#include <numeric>

#include "CurveFitting.h"
#include "../Base/ThreadPool.h"
#include "../QMapPropsDialog.h"
#include "alglib/fasttransforms.h"

const QStringList &CurveFitting::implementations()
{
    static QStringList g_impl{"Asymmetric Gaussian", "Parabola"};
    return g_impl;
}

CurveFitting::Ptr CurveFitting::create(const QString &name, const DoubleVector &x, const DoubleVector &y)
{
    if(name == "Asymmetric Gaussian")
        return Ptr(new AsymmetricGaussian(x, y));
    if(name == "Parabola")
        return Ptr(new Parabola(x, y));
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
    try {
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
    }
    catch (const cv::Exception& ex)
    {
        QMessageBox::warning(Q_NULLPTR, QObject::tr("OpenCV exception handler"), QString("Exception") + ex.what());
        init(x, y);
    }
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
    mParams->mA = std::isnormal(A / norm) ? A / norm : * std::max_element(y.begin(), y.end());
}

void AsymmetricGaussian::estimateErrors
(
    const CurveFitting::DoubleVector &x
)
{
    try {
        const size_t nRuns = 100;
        DoubleVector y0;
        values(x, y0);
        DoubleVector  ty(y0.size());
        std::poisson_distribution<> dist;
        std::mt19937_64 gen;
        double sig = 0.0;
        for(size_t i = 0; i < nRuns; ++i)
        {
            for(size_t j = 0; j < y0.size(); ++j)
            {
                if(y0[j] > 0.0)
                {
                    dist.param(std::poisson_distribution<>::param_type(y0[j]));
                    ty[j] = dist(gen);
                }
                else
                {
                    ty[j] = 0.0;
                }
            }
            AsymmetricGaussian gaus(x, ty, *this);
            double d = mParams->mTc - gaus.mParams->mTc;
            sig += d * d;
        }
        sig /= nRuns;
        mErrors->mTc = std::sqrt(sig);
    } catch (const std::exception& ex) {
        QMessageBox::warning(Q_NULLPTR, "std::exception handler", ex.what());
    }
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
    DoubleVector yy;
    mObj->values(m_x, yy);
    for (size_t i = 0; i < m_x.size(); ++i)
    {
        double ds = yy[i] - m_y[i];
        s += ds * ds;
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

Parabola::Parabola(const CurveFitting::DoubleVector &x, const CurveFitting::DoubleVector &y)
    :
      CurveFitting (x, y)
{
    Eigen::VectorXd ty(x.size());
    x0 = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    Eigen::MatrixXd M(x.size(), 3);
    for(size_t i = 0; i < x.size(); ++i)
    {
        ty(static_cast<Eigen::Index>(i)) = y[i];
        double xx = x[i] - x0;
        M(static_cast<Eigen::Index>(i), 0) = 1.;
        M(static_cast<Eigen::Index>(i), 1) = xx;
        M(static_cast<Eigen::Index>(i), 2) = xx * xx;
    }
    Eigen::MatrixXd A = M.transpose() * M;
    Eigen::VectorXd bb = M.transpose() * ty;
    Eigen::VectorXd coefs = A.ldlt().solve(bb);
    a = coefs(2); b = coefs(1); c = coefs(0);
    DoubleVector yy;
    values(x, yy);
    double ss = std::inner_product
    (
        y.begin(),
        y.end(),
        yy.begin(),
        0.0,
        std::plus<double>(),
        [](double a, double b)->double
    {
        return (a - b) * (a - b);
    }
    ) / x.size();
    A = A.inverse() * ss;
    sa = std::sqrt(A(2,2));
    sb = std::sqrt(A(1,1));
    sc = std::sqrt(A(0,0));
}

void Parabola::values(const CurveFitting::DoubleVector &x, CurveFitting::DoubleVector &y) const
{
    y.resize(x.size());

    ThreadPool::parFor(x.size(), [&](size_t i)
    {
        double xx = x[i] - x0;
        y[i] = (a*xx + b)*xx + c;
    });
}

QString Parabola::eqn() const
{
    return "(a*x + b)*x + c";
}

CurveFitting::ParamsList Parabola::params() const
{
    return ParamsList
    {
        {"a", a},
        {"b", b},
        {"c", c}
    };
}

void Parabola::setParams(const CurveFitting::ParamsList &pars)
{
    ParamsList::ConstIterator it = pars.find("a");
    bool ok = true;
    if(it != pars.end()) a = it.value().toDouble(&ok);
    if((it = pars.find("b")) != pars.end()) b = it.value().toDouble(&ok);
    if((it = pars.find("c")) != pars.end()) c = it.value().toDouble(&ok);
    Q_ASSERT(ok);
}

CurveFitting::ParamsList Parabola::errors() const
{
    return ParamsList
    {
        {"a", sa},
        {"b", sb},
        {"c", sc}
    };
}

CurveFitting::ParamsList Parabola::properties() const
{
    return ParamsList();
}

void Parabola::setProperties(const CurveFitting::ParamsList& /**/)
{

}

void Parabola::print(QTextStream &out) const
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

double Parabola::peakPosition() const
{
    return x0 - b / 2. / a;
}

double Parabola::peakPositionUncertainty() const
{
    double
            d1 = sb / 2. / a,
            d2 = sa * b / 2. / a / a;
    return std::sqrt(d1*d1 + d2*d2);
}

PeakShapeFit::PeakShapeFit(const CurveFitting::DoubleVector &x, const CurveFitting::DoubleVector &y)
    :
      CurveFitting (x, y),
      mShape(new InterpolatorFun),
      mRelTol(1.e-9),
      mPeakPositionUncertainty(0.0)
{
    double fPeakPosition = x[0] + maxPeakPos(y);
    DoubleVector tx = x;
    for(double & xx : tx) xx -= fPeakPosition;
    mShape->setXYValues(tx, y);
    mShape->setPeakAmp(1.0);
    mShape->setPeakWidth(1.0);
    mShape->setPeakPosition(fPeakPosition);
    calculateUncertainty(x, 100);
}

void PeakShapeFit::values(const CurveFitting::DoubleVector &x, CurveFitting::DoubleVector &y) const
{
    y = mShape->values(x);
}

QString PeakShapeFit::eqn() const
{
    return "Peak shape fit";
}

CurveFitting::ParamsList PeakShapeFit::params() const
{
    return ParamsList
    {
        {"Position", mShape->peakPosition()},
        {"Width", mShape->peakWidth()},
        {"Amplitude", mShape->peakAmp()}
    };
}

void PeakShapeFit::setParams(const CurveFitting::ParamsList &pars)
{
    ParamsList::ConstIterator it = pars.find("Position");
    bool ok = true;
    if(it != pars.end())
    {
        mShape->setPeakPosition(it.value().toDouble(&ok));
    }
    if((it = pars.find("Width")) != pars.end())
    {
        mShape->setPeakWidth(it.value().toDouble(&ok));
    }
    if((it = pars.find("Amplitude")) != pars.end())
    {
        mShape->setPeakAmp(it.value().toDouble(&ok));
    }
    Q_ASSERT(ok);
}

CurveFitting::ParamsList PeakShapeFit::errors() const
{
    return ParamsList
    {
        {"Position", peakPositionUncertainty()},
        {"Width", 0.0},
        {"Amplitude", 0.0}
    };
}

CurveFitting::ParamsList PeakShapeFit::properties() const
{
    return ParamsList{ {"TOL", mRelTol} };
}

void PeakShapeFit::setProperties(const CurveFitting::ParamsList &props)
{
    ParamsList::ConstIterator it = props.find("TOL");
    bool ok = true;
    if(it != props.end()) mRelTol = it.value().toDouble(&ok);
    Q_ASSERT(ok);
}

void PeakShapeFit::print(QTextStream &out) const
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

double PeakShapeFit::peakPosition() const
{
    return mShape->peakPosition();
}

double PeakShapeFit::peakPositionUncertainty() const
{
    return mPeakPositionUncertainty;
}

void PeakShapeFit::fit(const CurveFitting::DoubleVector &x, const CurveFitting::DoubleVector &y)
{
    QMapPropsDialog dialog;
    dialog.setProps(properties());
    dialog.exec();
    setProperties(dialog.props());
    cv::Mat_<double> res(2, 1);
    double newPeakPos = x[0] + maxPeakPos(y);
    mShape->setPeakPosition(newPeakPos);
    DoubleVector yy = mShape->values(x);
    //first approximation for amplitude
    double A = 0.0, ysum = 0.0;
    for(size_t i = 0; i < y.size(); ++i)
    {
        A += y[i] * yy[i];
        ysum += yy[i] * yy[i];
    }
    ysum != 0. ? A /= ysum / mShape->peakAmp() : A = 0;
    res << A, newPeakPos;
    cv::Ptr<cv::DownhillSolver> solver
    (
        cv::DownhillSolver::create
        (
            cv::Ptr<Function>(new Function(this, x, y))
        )
    );
    cv::Mat_<double> step = 0.1 * res;
    step(1) = 1;
    solver->setInitStep(step);
    solver->setTermCriteria(cv::TermCriteria(3, 10000, mRelTol));
    solver->minimize(res);
    mShape->setPeakAmp(res(0));
    mShape->setPeakPosition(res(1));
    calculateUncertainty(x, 100);
}

void PeakShapeFit::import(QTextStream &out) const
{
    mShape->import(out);
}

InterpolatorFun PeakShapeFit::cloneShape() const
{
    return InterpolatorFun(*mShape);
}

CurveFitting::DoubleVector PeakShapeFit::crossCorrelate
(
    const DoubleVector &x,
    const DoubleVector &y,
    int nPeaks
) const
{
    DoubleVector peaks = crossCorrPeaks(x, y, nPeaks);
    DoubleVector uncertainties = crossUncertainty
    (
        x,
        crossSignal(x, peaks, calcAmps(x, y, peaks)),
        peaks
    );
    peaks.insert(peaks.end(), uncertainties.begin(), uncertainties.end());
    return peaks;
}

CurveFitting::DoubleVector PeakShapeFit::crossCorrPeaks
(
    const DoubleVector &x,
    const DoubleVector &y,
    int nPeaks
) const
{
    DoubleVector pattern;
    values(x, pattern);
    alglib::real_1d_array s, p, r;
    const alglib::ae_int_t n = x.size();
    s.setlength(n); p.setlength(n);
    s.setcontent(n, y.data());
    p.setcontent(n, pattern.data());
    r.setlength(2*n - 1);
    alglib::corrr1d(s, n, p, n, r);
    std::rotate
    (
        r.c_ptr()->ptr.p_double,
        r.c_ptr()->ptr.p_double + n,
        r.c_ptr()->ptr.p_double + r.length()
    );
    using VectorPair = std::vector<std::pair<double, double>>;
    VectorPair peaks;
    DoubleVector xx(2 * n - 1, 0);
    std::iota(xx.begin(), xx.end(), -(n - 1));
    const double h = x[1] - x[0];
    for(alglib::ae_int_t i = 1; i < r.length() - 1; ++i)
    {
        if(r(i-1) < r(i) && r(i+1) < r(i))
        {
            const double a = r(i+1) - 2.*r(i) + r(i-1);
            const double b = r(i+1) - r(i-1);
            peaks.push_back
            (
                {
                peakPosition() + (xx[i] - b/2./a) * h,
                (r(i) - b*b/4./a)
                }
            );
        }
    }
    VectorPair::iterator itEnd
            = std::remove_if
    (
        peaks.begin(),
        peaks.end(),
        [&](const std::pair<double, double>& p)->bool
    {
        return p.first <= x.front() && p.first >= x.back();
    }
    );
    peaks.assign(peaks.begin(), itEnd);
    std::sort
    (
        peaks.begin(),
        peaks.end(),
        [](VectorPair::const_reference a, VectorPair::const_reference b)->bool
    {
        return a.second > b.second;
    }
    );
    peaks.resize(nPeaks);
    DoubleVector res(nPeaks);
    std::transform
    (
        peaks.begin(),
        peaks.end(),
        res.begin(),
        [](VectorPair::const_reference a)->double
    {
        return a.first;
    }
    );
    std::sort(res.begin(), res.end(), std::less<>());
    return res;
}

CurveFitting::DoubleVector PeakShapeFit::calcAmps
(
    const DoubleVector &x,
    const DoubleVector &y,
    const DoubleVector &peaks
) const
{
    const double t0 = mShape->peakPosition();
    Eigen::MatrixXd M(x.size(), peaks.size());
    Eigen::VectorXd Y(y.size());
    DoubleVector yy;
    for(size_t i = 0; i < x.size(); ++i) Y(i) = y[i];
    for(size_t i = 0; i < peaks.size(); ++i)
    {
        mShape->setPeakPosition(peaks[i]);
        yy = mShape->values(x);
        for(size_t j = 0; j < x.size(); ++j)
        {
            M(j, i) = yy[j];
        }
    }
    Eigen::VectorXd A = (M.transpose()*M).ldlt().solve(M.transpose()*Y);
    mShape->setPeakPosition(t0);
    return DoubleVector(A.array().data(), A.array().data() + A.size());
}

CurveFitting::DoubleVector PeakShapeFit::crossSignal
(
    const DoubleVector &x,
    const DoubleVector &peaks,
    const DoubleVector &amps
) const
{
    const double A = mShape->peakAmp();
    const double t0 = mShape->peakPosition();
    DoubleVector yy(x.size(), 0.0);
    for(size_t i = 0; i < peaks.size(); ++i)
    {
        mShape->setPeakAmp(amps[i]);
        mShape->setPeakPosition(peaks[i]);
        DoubleVector dy = mShape->values(x);
        for(size_t j = 0; j < x.size(); ++j)
        {
            yy[j] += dy[j];
        }
    }
    mShape->setPeakAmp(A);
    mShape->setPeakPosition(t0);
    return yy;
}

CurveFitting::DoubleVector PeakShapeFit::crossUncertainty
(
    const DoubleVector &x,
    const DoubleVector &y,
    const DoubleVector &peaks
) const
{
    DoubleVector res(peaks.size(), 0.0);
    std::poisson_distribution<> dist;
    std::mt19937_64 gen;
    DoubleVector yy(y.size()), peaks1(peaks.size());
    for(int i = 0; i < 100; ++i)
    {
        for(size_t i = 0; i < y.size(); ++i)
        {
            if(y[i] > .0)
            {
                dist.param(std::poisson_distribution<>::param_type(y[i]));
                yy[i] = dist(gen);
            }
            else
            {
                yy[i] = 0.0;
            }
        }
        peaks1 = crossCorrPeaks(x, yy, peaks.size());
        for(size_t j = 0; j < peaks.size(); ++j)
        {
            const double d = peaks1[j] - peaks[j];
            res[j] += d*d;
        }
    }
    for(size_t j = 0; j < peaks.size(); ++j) res[j] = std::sqrt(res[j]/100);
    return res;
}

double PeakShapeFit::maxPeakPos(const CurveFitting::DoubleVector &y)
{
    DoubleVector::const_iterator it = std::max_element(y.cbegin(), y.cend());
    const size_t n = static_cast<size_t>(std::distance(y.cbegin(), it));
    double b = y[n+1] - y[n-1];
    double a = y[n+1] - 2*y[n] + y[n-1];
    return static_cast<double>(n) - b / 2 / a;
}

void PeakShapeFit::calculateUncertainty(const DoubleVector& vXVals, const int nRuns)
{
    double fPeakPosition = mShape->peakPosition();
    double fAmp = mShape->peakAmp();
    DoubleVector y = mShape->values(vXVals), ty(y.size());
    std::poisson_distribution<> dist;
    std::mt19937_64 gen;
    double s = 0.0;
    for(int i = 0; i < nRuns; ++i)
    {
        for(size_t j = 0; j < y.size(); ++j)
        {
            if(y[j] > 0.0)
            {
                dist.param(std::poisson_distribution<>::param_type(y[j]));
                ty[j] = dist(gen);
            }
            else
            {
                ty[j] = 0.0;
            }
        }
        cv::Mat_<double> res(2,1);
        res << mShape->peakAmp(), fPeakPosition;
        cv::Ptr<cv::DownhillSolver> solver
        (
            cv::DownhillSolver::create
            (
                cv::Ptr<Function>(new Function(this, vXVals, ty))
            )
        );
        cv::Mat_<double> step = 0.1 * res;
        step(1) = 1;
        solver->setInitStep(step);
        solver->setTermCriteria(cv::TermCriteria(3, 10000, mRelTol));
        solver->minimize(res);

        double d = res(1) - fPeakPosition;
        s += d*d;
    }
    mPeakPositionUncertainty = std::sqrt(s/nRuns);
    mShape->setPeakAmp(fAmp);
    mShape->setPeakPosition(fPeakPosition);
}

int PeakShapeFit::Function::getDims() const
{
    return 2;
}

double PeakShapeFit::Function::calc(const double *x) const
{
    double ss = 0.0;
    DoubleVector yy(m_x.size());
    mObj->mShape->setPeakAmp(x[0]);
    mObj->mShape->setPeakPosition(x[1]);
    mObj->values(m_x, yy);
    for (size_t i = 0; i < m_x.size(); ++i)
    {
        double ds = (m_y[i] - yy[i]);
        ss += ds * ds;
    }
    return ss;
}

DoublePeakShapeFit::DoublePeakShapeFit(const PeakShapeFit &onePeakShape, const DoubleVector& x, const DoubleVector& y)
    :
      mShape1(new InterpolatorFun(onePeakShape.cloneShape())),
      mShape2(new InterpolatorFun(onePeakShape.cloneShape())),
      mPeakPositionUncertainty1(0.0),
      mPeakPositionUncertainty2(0.0)
{
    QMapPropsDialog dialog;
    QVariantMap props{{"t1: ", .0}, {"t2: ", .0}};
    dialog.setProps(props);
    dialog.exec();
    props = dialog.props();
    bool ok = true;
    mShape1->setPeakPosition(props["t1: "].toDouble(&ok));
    mShape2->setPeakPosition(props["t2: "].toDouble(&ok));
    Q_ASSERT(ok);

    calcAmps(x, y);
    fit(x, y);
}

void DoublePeakShapeFit::values(const DoublePeakShapeFit::DoubleVector &x, DoublePeakShapeFit::DoubleVector &y) const
{
    y = mShape1->values(x);
    DoubleVector dy = mShape2->values(x);
    for (size_t i = 0; i < y.size(); ++i) y[i] += dy[i];
}

void DoublePeakShapeFit::fit(const DoubleVector& x, const DoubleVector& y)
{
    mPeakPositionUncertainty1 = 0.0;
    mPeakPositionUncertainty2 = 0.0;
    cv::Mat_<double> p0{mShape1->peakPosition(), mShape2->peakPosition()};
    minimize(p0, p0, Function(this, x, y));
    mShape1->setPeakPosition(p0(0));
    mShape2->setPeakPosition(p0(1));
    calcAmps(x, y);
    DoubleVector yy;
    values(x, yy);
    DoubleVector ty(y.size());
    std::poisson_distribution<> dist;
    std::mt19937_64 gen;
    double fMax1 = p0(0); double fMax2 = p0(1);
    for(int i = 0; i < 100; ++i)
    {
        p0 = cv::Mat_<double>{fMax1, fMax2};
        for(size_t j = 0; j < yy.size(); ++j)
        {
            if(yy[j] > 0.)
            {
                dist.param(std::poisson_distribution<>::param_type(yy[j]));
                ty[j] = dist(gen);
            }
            else
            {
                ty[j] = 0.;
            }
        }
        minimize(p0, p0, Function(this, x, ty));
        double dp1 = fMax1 - p0(0);
        mPeakPositionUncertainty1 += dp1 * dp1;
        double dp2 = fMax2 - p0(1);
        mPeakPositionUncertainty2 += dp2 * dp2;
    }
    mPeakPositionUncertainty1 = std::sqrt(mPeakPositionUncertainty1 / 100);
    mPeakPositionUncertainty2 = std::sqrt(mPeakPositionUncertainty2 / 100);
}

double DoublePeakShapeFit::peakPositionUncertainty2() const
{
    return mPeakPositionUncertainty2;
}

double DoublePeakShapeFit::peakPositionUncertainty1() const
{
    return mPeakPositionUncertainty1;
}

void DoublePeakShapeFit::minimize(const cv::Mat_<double> &p0, cv::Mat_<double> &p1, const Function &fun)
{
    cv::Ptr<cv::DownhillSolver> solver
    (
        cv::DownhillSolver::create
        (
            cv::Ptr<Function>(new Function(fun))
        )
    );
    p1 = p0;
    cv::Mat_<double> step{.1, .1};
    solver->setInitStep(step);
    solver->setTermCriteria(cv::TermCriteria(3, 10000, 1e-9));
    solver->minimize(p1);
}

void DoublePeakShapeFit::calcAmps(const DoubleVector &x, const DoubleVector &y)
{
    Eigen::MatrixXd M(y.size(), 2);
    Eigen::VectorXd Y(y.size());
    DoubleVector y1 = mShape1->values(x);
    DoubleVector y2 = mShape2->values(x);

    for(size_t i = 0; i < y.size(); ++i)
    {
        M(i, 0) = y1[i];
        M(i, 1) = y2[i];
        Y(i) = y[i];
    }

    Eigen::VectorXd A = (M.transpose() * M).ldlt().solve(M.transpose() * Y);

    mShape1->setPeakAmp(mShape1->peakAmp() * A(0));
    mShape2->setPeakAmp(mShape2->peakAmp() * A(1));
}

int DoublePeakShapeFit::Function::getDims() const
{
    return 2;
}

double DoublePeakShapeFit::Function::calc(const double *x) const
{
    double ss = 0.0;
    DoubleVector yy(m_x.size());
    mObj->mShape1->setPeakPosition(x[0]);
    mObj->mShape2->setPeakPosition(x[1]);
    mObj->calcAmps(m_x, m_y);
    mObj->values(m_x, yy);
    for (size_t i = 0; i < m_x.size(); ++i)
    {
        double ds = (m_y[i] - yy[i]);
        ss += ds * ds;
    }
    return ss;
}

MultiShapeFit::MultiShapeFit
(
    const PeakShapeFit &onePeakShape,
    const DoubleVector &x,
    const DoubleVector &y,
    size_t nShapes
)
    :
      mShapes(nShapes),
      mUncertainties(nShapes)
{
    QVariantMap props;
    for(size_t i = 0; i < nShapes; ++i)
    {
        mShapes[i].reset(new InterpolatorFun(onePeakShape.cloneShape()));
        props[QString("t%1: ").arg(i)] = 0.;
    }
    QMapPropsDialog dialog;
    dialog.setProps(props);
    dialog.exec();
    props = dialog.props();
    for(size_t i = 0; i < nShapes; ++i)
    {
        mShapes[i]->setPeakPosition(props[QString("t%1: ").arg(i)].toDouble());
    }
    setWidth(1.0);
    if(calcAmps(x, y)) fit(x, y);
}

void MultiShapeFit::values
(
    const DoubleVector &x,
    DoubleVector &y
) const
{
    y.assign(x.size(), 0.0);
    DoubleVector dy(x.size());
    for(size_t j = 0; j < mShapes.size(); ++j)
    {
        dy = mShapes[j]->values(x);
        for(size_t i = 0; i < y.size(); ++i)
        {
            y[i] += dy[i];
        }
    }
}

double MultiShapeFit::value(double x) const
{
    double res = .0;
    for(size_t j = 0; j < mShapes.size(); ++j)
    {
        res += mShapes[j]->value(x);
    }
    return res;
}

void MultiShapeFit::fit(const MultiShapeFit::DoubleVector &x, const MultiShapeFit::DoubleVector &y)
{
    mUncertainties.assign(mUncertainties.size(), 0.0);
    minimize(Function(this, x, y));
    std::poisson_distribution<> dist;
    std::mt19937_64 gen;
    DoubleVector tp(mShapes.size()), tA(mShapes.size());
    for(size_t i = 0; i < mShapes.size(); ++i)
    {
        tp[i] = mShapes[i]->peakPosition();
        tA[i] = mShapes[i]->peakAmp();
    }
    double tw = mW;
    DoubleVector yy(x.size()), ty(yy.size());
    for(int i = 0; i < 100; ++i)
    {
        values(x, yy);
        for(size_t j = 0; j < x.size(); ++j)
        {
            if(yy[j] > 0.)
            {
                dist.param(std::poisson_distribution<>::param_type(yy[j]));
                ty[j] = dist(gen);
            }
            else
            {
                ty[j] = .0;
            }
        }
        minimize(Function(this, x, ty));
        for(size_t j = 0; j < mShapes.size(); ++j)
        {
            const double d = tp[j] - mShapes[j]->peakPosition();
            mUncertainties[j] += d*d;
            mShapes[j]->setPeakPosition(tp[j]);
            mShapes[j]->setPeakAmp(tA[j]);
        }
        setWidth(tw);
    }
    for(double& u : mUncertainties)
    {
        u = std::sqrt(u / 100);
    }
    calcAmps(x, y);
}

void MultiShapeFit::importData(QTextStream &out) const
{
    for(size_t i = 0; i < mShapes.size(); ++i)
    {
        out.setRealNumberPrecision(10);
        out << mShapes[i]->peakPosition() / 10. << "\t";
        out.setRealNumberPrecision(3);
        out << mUncertainties[i] / 10. << "\n";
    }
}

void MultiShapeFit::minimize(const MultiShapeFit::Function &fun)
{
    cv::Ptr<cv::DownhillSolver> solver
    (
        cv::DownhillSolver::create
        (
            cv::Ptr<Function>(new Function(fun))
        )
    );
    cv::Mat_<double> p0(2*mShapes.size() + 1, 1);
    for(size_t i = 0; i < mShapes.size(); ++i)
    {
        p0(i, 0) = mShapes[i]->peakPosition();
        p0(i + mShapes.size(), 0) = mShapes[i]->peakAmp();
    }
    p0(2*mShapes.size(), 0) = mW;
    cv::Mat_<double> step(2*mShapes.size() + 1, 1);
    for(size_t i = 0; i < mShapes.size(); ++i)
    {
        step(i, 0) = 0.1;
        step(i + mShapes.size(), 0) = 0.01 * mShapes[i]->peakAmp();
    }
    step(2 * mShapes.size(), 0) = 0.01 * mW;
    solver->setInitStep(step);
    solver->setTermCriteria(cv::TermCriteria(3, 1000, 1e-9));
    solver->minimize(p0);
    setWidth(p0(2*mShapes.size(), 0));
    for(size_t i = 0; i < mShapes.size(); ++i)
    {
        mShapes[i]->setPeakPosition(p0(i, 0));
        mShapes[i]->setPeakAmp(p0(i + mShapes.size(), 0));
    }
}

void MultiShapeFit::setWidth(double w)
{
    for(auto p : mShapes)
    {
        p->setPeakWidth(w);
    }
    mW = w;
}

bool MultiShapeFit::calcAmps
(
    const DoubleVector &x,
    const DoubleVector &y
)
{
    Eigen::MatrixXd M(x.size(), mShapes.size());
    Eigen::VectorXd B(x.size());
    DoubleVector ty(x.size());
    for(size_t i = 0; i < x.size(); ++i)
    {
        B(i) = y[i];
    }
    for(size_t j = 0; j < mShapes.size(); ++j)
    {
         ty = mShapes[j]->values(x);
         for(size_t i = 0; i < x.size(); ++i)
         {
             M(i, j) = ty[i];
         }
    }
    Eigen::VectorXd Amps = (M.transpose() * M).ldlt().solve(M.transpose() * B);
    for(size_t i = 0; i < mShapes.size(); ++i)
    {
        mShapes[i]->setPeakAmp(Amps(i) * mShapes[i]->peakAmp());
    }
    return std::all_of(mShapes.begin(), mShapes.end(), [](const auto& p)->bool{ return p->peakAmp() > 0.; });
}

int MultiShapeFit::Function::getDims() const
{
    return 2 * mObj->mShapes.size() + 1;
}

double MultiShapeFit::Function::calc(const double *x) const
{
    for(size_t i = 0; i < mObj->mShapes.size(); ++i)
    {
        mObj->mShapes[i]->setPeakPosition(x[i]);
        mObj->mShapes[i]->setPeakAmp(x[i + mObj->mShapes.size()]);
    }
    mObj->setWidth(x[2*mObj->mShapes.size()]);
    double s = 0.;
    for(size_t i = 0; i < m_x.size(); ++i)
    {
        const double y0 = mObj->value(m_x[i]);
        const double ds = (m_y[i] - y0);
        s += ds * ds / (y0 + 1.);
    }
    return s;
}
