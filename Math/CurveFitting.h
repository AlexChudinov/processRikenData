#ifndef CURVEFITTING_H
#define CURVEFITTING_H

#include <QTextStream>
#include <vector>
#include <QString>
#include <QVariant>
#include <QMap>

class CurveFitting
{
public:
    using DoubleVector = std::vector<double>;
    using ParamsList = QVariantMap;

    CurveFitting(const DoubleVector& x, const DoubleVector& y);

    virtual ~CurveFitting();

    virtual void values(const DoubleVector& x, DoubleVector& y) const = 0;

    /**
     * @brief egn returns equation that was used for fitting
     * @return
     */
    virtual QString egn() const = 0;

    /**
     * @brief params current parameter values
     * @return
     */
    virtual ParamsList params() const = 0;
    virtual void setParams(const ParamsList&) = 0;

    virtual QTextStream& operator<<(QTextStream& out) const = 0;
};

/**
 * @brief The AsymetricExponential class fits data with gaussian
 * with assymetric exponential tails
 */
class AsymmetricGaussian : CurveFitting
{
    struct Parameters
    {
        double mDTL;
        double mDTR;
        double mTc;
        double mW;
    };

    struct Errors
    {
        double mDTL;
        double mDTR;
        double mTc;
        double mW;
    };

public:
    AsymmetricGaussian(const DoubleVector& x, const DoubleVector& y);

    virtual void values(const DoubleVector& x, DoubleVector& y) const;

    QString eqn() const;

    ParamsList params() const;
    void setParams(const ParamsList &params);


private:

    QScopedPointer<Parameters> mParams;
    QScopedPointer<Errors> mErrors;

    inline void init(const DoubleVector& x, const DoubleVector& y);

    inline double value(double x) const;
};

void AsymmetricGaussian::init
(
    const DoubleVector& x,
    const DoubleVector& y
)
{
    mParams.reset(new Parameters);
    mErrors.reset(new Errors);
    *mParams = {0., 0., 0., 0.};
    *mErrors = {0., 0., 0., 0.};
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
}

double AsymmetricGaussian::value(double x) const
{
    double dx = (x - mParams->mTc) / mParams->mW;
    double dxL = mParams->mDTL / mParams->mW;
    double dxR = mParams->mDTR / mParams->mW;
    if(dx < - dxL)
    {
        return std::exp(dxL * dx + .5 * dxL * dxL);
    }
    else if(dx > dxR)
    {
        return std::exp(.5 * dxR * dxR - dxR * dx);
    }
    else return std::exp(-.5 * dx * dx);
}

#endif // CURVEFITTING_H
