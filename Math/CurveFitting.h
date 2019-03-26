#ifndef CURVEFITTING_H
#define CURVEFITTING_H

#include <memory>
#include <QTextStream>
#include <vector>
#include <QString>
#include <QVariant>
#include <QMap>
#include <opencv2/core/core.hpp>

class CurveFitting
{
public:
    using DoubleVector = std::vector<double>;
    using ParamsList = QVariantMap;
    using Ptr = std::unique_ptr<CurveFitting>;

    static const QStringList& implementations();

    static Ptr create(const QString& name, const DoubleVector& x, const DoubleVector& y);

    CurveFitting(const DoubleVector& x, const DoubleVector& y);

    virtual ~CurveFitting();

    virtual void values(const DoubleVector& x, DoubleVector& y) const = 0;

    /**
     * @brief egn returns equation that was used for fitting
     * @return
     */
    virtual QString eqn() const = 0;

    /**
     * @brief params current parameter values
     * @return
     */
    virtual ParamsList params() const = 0;
    virtual void setParams(const ParamsList&) = 0;

    virtual ParamsList errors() const = 0;

    /**
     * @brief operator << prints fitting data into stream
     * @param out
     * @return
     */
    virtual QTextStream& operator<<(QTextStream& out) const = 0;
};

namespace alglib {
    class real_1d_array;
}

/**
 * @brief The AsymetricExponential class fits data with gaussian
 * with assymetric exponential tails
 */
class AsymmetricGaussian : public CurveFitting
{
    friend class Function;

    struct Parameters
    {
        double mA;
        double mDTL;
        double mDTR;
        double mTc;
        double mW;
    };

    class Function : public cv::MinProblemSolver::Function
    {
        mutable AsymmetricGaussian * mObj;
        const DoubleVector& m_x;
        const DoubleVector& m_y;
        virtual int getDims() const;
        virtual double calc(const double* x) const;
    public:
        Function
        (
            AsymmetricGaussian * obj,
            const DoubleVector& x,
            const DoubleVector& y
        )
            :
              cv::MinProblemSolver::Function(),
              mObj(obj),
              m_x(x),
              m_y(y)
        {

        }
    };

    using Errors = Parameters;

public:
    AsymmetricGaussian(const DoubleVector& x, const DoubleVector& y);

    virtual void values(const DoubleVector& x, DoubleVector& y) const;

    QString eqn() const;

    ParamsList params() const;
    void setParams(const ParamsList &params);
    ParamsList errors() const;

    virtual QTextStream& operator<<(QTextStream& out) const;
private:

    QScopedPointer<Parameters> mParams;
    QScopedPointer<Errors> mErrors;

    void init(const DoubleVector& x, const DoubleVector& y);

    inline double value(double x) const;

    /**
     * @brief curveScaling recalculates curve scaling factor
     */
    void curveScaling(const DoubleVector& x, const DoubleVector& y);
};

double AsymmetricGaussian::value(double x) const
{
    const double dx = (x - mParams->mTc) / mParams->mW;
    const double dxL = mParams->mDTL / mParams->mW;
    const double dxR = mParams->mDTR / mParams->mW;
    if(dx < - dxL)
        return mParams->mA * std::exp(dxL * dx + .5 * dxL * dxL);
    else if(dx > dxR)
        return mParams->mA * std::exp(.5 * dxR * dxR - dxR * dx);
    else
        return mParams->mA * std::exp(-.5 * dx * dx);
}

#endif // CURVEFITTING_H
