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

    virtual ParamsList properties() const = 0;
    virtual void setProperties(const ParamsList&) = 0;

    /**
     * @brief operator << prints fitting data into stream
     * @param out
     * @return
     */
    virtual QTextStream& operator>>(QTextStream& out) const = 0;
};

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

    struct Properties
    {
        double mStep;
        double mRelTol;
        double mIterNum;
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

    ParamsList properties() const;
    void setProperties(const ParamsList& props);

    virtual QTextStream& operator>>(QTextStream& out) const;
private:
    double mResudials;

    QScopedPointer<Parameters> mParams;
    QScopedPointer<Errors> mErrors;
    QScopedPointer<Properties> mProps;

    void init(const DoubleVector& x, const DoubleVector& y);

    inline double value(double x) const;

    /**
     * @brief curveScaling recalculates curve scaling factor
     */
    void curveScaling(const DoubleVector& x, const DoubleVector& y);

    void estimateErrors(const DoubleVector& x, const DoubleVector& y);

    //Derivatives of function by its parameters
    double dfdA(double x) const;
    double dfdw(double x) const;
    double dfdtL(double x) const;
    double dfdtR(double x) const;
    double dfdtc(double x) const;

    double residuals(const DoubleVector& x, const DoubleVector& y) const;
};

double AsymmetricGaussian::value(double x) const
{
    return mParams->mA * dfdA(x);
}

#endif // CURVEFITTING_H
