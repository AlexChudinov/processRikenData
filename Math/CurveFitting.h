#ifndef CURVEFITTING_H
#define CURVEFITTING_H

#include <memory>
#include <QTextStream>
#include <vector>
#include <QString>
#include <QVariant>
#include <QMap>
#include <opencv2/core/core.hpp>
#include "Math/peakparams.h"
#include "../Data/PeakShape.h"

class CurveFitting : public PeakParams
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
     * @brief prints fitting data into stream
     * @param out
     * @return
     */
    virtual void print(QTextStream& out) const = 0;
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

        int getDims() const;
        double calc(const double* x) const;
        void getGradient(const double * x, double * y);
    };

    using Errors = Parameters;

    AsymmetricGaussian
    (
        const DoubleVector& x,
        const DoubleVector& y,
        const AsymmetricGaussian& other
    );

public:
    AsymmetricGaussian(const DoubleVector& x, const DoubleVector& y);

    virtual void values(const DoubleVector& x, DoubleVector& y) const;

    QString eqn() const;

    ParamsList params() const;
    void setParams(const ParamsList &params);
    ParamsList errors() const;

    ParamsList properties() const;
    void setProperties(const ParamsList& props);

    virtual void print(QTextStream& out) const;

    double peakPosition() const;
    double peakPositionUncertainty() const;

private:
    double mResudials;

    QScopedPointer<Parameters> mParams;
    QScopedPointer<Errors> mErrors;
    QScopedPointer<Properties> mProps;

    double run(const DoubleVector& x, const DoubleVector& y);

    void init(const DoubleVector& x, const DoubleVector& y);

    inline double value(double x) const;

    /**
     * @brief curveScaling recalculates curve scaling factor
     */
    void curveScaling(const DoubleVector& x, const DoubleVector& y);

    void estimateErrors(const DoubleVector& x);

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

/**
 * @brief The Parabola class fitts peak apex with parabola
 */
class Parabola : public CurveFitting
{
    double x0;
    double a, b, c, sa, sb, sc;
public:
    Parabola(const DoubleVector& x, const DoubleVector& y);

    virtual void values(const DoubleVector& x, DoubleVector& y) const;

    QString eqn() const;

    ParamsList params() const;
    void setParams(const ParamsList &pars);
    ParamsList errors() const;

    ParamsList properties() const;
    void setProperties(const ParamsList& props);

    virtual void print(QTextStream& out) const;

    double peakPosition() const;
    double peakPositionUncertainty() const;
};

class InterpolatorFun;

class PeakShapeFit : public CurveFitting
{
    std::unique_ptr<InterpolatorFun> mShape;
    class Function : public cv::MinProblemSolver::Function
    {
        mutable PeakShapeFit * mObj;
        const DoubleVector& m_x;
        const DoubleVector& m_y;
    public:
        Function
        (
            PeakShapeFit * obj,
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

        int getDims() const;
        double calc(const double* x) const;
    };

public:
    PeakShapeFit(const DoubleVector &x, const DoubleVector &y);

    virtual void values(const DoubleVector& x, DoubleVector& y) const;

    QString eqn() const;

    ParamsList params() const;
    void setParams(const ParamsList &pars);
    ParamsList errors() const;

    ParamsList properties() const;
    void setProperties(const ParamsList& props);

    virtual void print(QTextStream& out) const;

    double peakPosition() const;
    double peakPositionUncertainty() const;

    /**
     * @brief fits shape to a new data
     * @param x
     * @param y
     */
    void fit(const DoubleVector& x, const DoubleVector& y);

    /**
     * @brief import peak shape
     * @param out
     */
    void import(QTextStream& out) const;

    InterpolatorFun cloneShape() const;

    /**
     * @brief crossCorrelate estimates peak shape cross-correlation
     * @param x
     * @param y
     * @param nPeaks
     * @return [peakPositions, peakUncertainties, peakAmplitudes]
     */
    DoubleVector crossCorrelate
    (
        const DoubleVector& x,
        const DoubleVector& y,
        int nPeaks = 1
    ) const;
    DoubleVector crossCorrPeaks
    (
        const DoubleVector& x,
        const DoubleVector& y,
        int nPeaks
    ) const;
    DoubleVector calcAmps
    (
        const DoubleVector& x,
        const DoubleVector& y,
        const DoubleVector& peaks
    ) const;
    DoubleVector crossSignal
    (
        const DoubleVector& x,
        const DoubleVector& peaks,
        const DoubleVector &amps
    ) const;
    DoubleVector crossUncertainty
    (
        const DoubleVector& x,
        const DoubleVector& y,
        const DoubleVector& peaks
    ) const;
private:
    static double maxPeakPos(const DoubleVector& y);
    void calculateUncertainty(const DoubleVector &vXVals, const int nRuns);
    double mRelTol;
    double mPeakPositionUncertainty;
};

/**
 * @brief The DoublePeakShapeFit class fits mass spectra region with a double shape
 */
class DoublePeakShapeFit
{
public:
    using DoubleVector = std::vector<double>;
private:
    class Function : public cv::MinProblemSolver::Function
    {
        mutable DoublePeakShapeFit * mObj;
        const DoubleVector& m_x;
        const DoubleVector& m_y;
        double mMaxY;
    public:
        Function
        (
            DoublePeakShapeFit * obj,
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

        int getDims() const;
        double calc(const double* x) const;
    };
public:
    DoublePeakShapeFit(const PeakShapeFit& onePeakShape, const DoubleVector &x, const DoubleVector &y);

    void values(const DoubleVector& x, DoubleVector& y) const;

    void fit(const DoubleVector& x, const DoubleVector& y);

    inline double peakPosition1() const { return mShape1->peakPosition(); }

    inline double peakPosition2() const { return mShape2->peakPosition(); }

    double peakPositionUncertainty2() const;

    double peakPositionUncertainty1() const;

private:
    std::unique_ptr<InterpolatorFun> mShape1, mShape2;
    double mPeakPositionUncertainty1, mPeakPositionUncertainty2;

    void minimize(const cv::Mat_<double>& p0, cv::Mat_<double>& p1, const Function& fun);

    void calcAmps(const DoubleVector& x, const DoubleVector& y);
};

class MultiShapeFit
{
public:
    using DoubleVector = DoublePeakShapeFit::DoubleVector;

private:
    class Function : public cv::MinProblemSolver::Function
    {
        mutable MultiShapeFit * mObj;
        const DoubleVector& m_x;
        const DoubleVector& m_y;
        double mMaxY;
    public:
        Function
        (
            MultiShapeFit * obj,
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

        int getDims() const;
        double calc(const double* x) const;

        const DoubleVector& xVals() const { return m_x; }
        const DoubleVector& yVals() const { return m_y; }
    };
public:
    MultiShapeFit
    (
        const PeakShapeFit& onePeakShape,
        const DoubleVector& x,
        const DoubleVector& y,
        size_t nShapes
    );

    void values(const DoubleVector& x, DoubleVector& y) const;

    double value(double x) const;

    void fit(const DoubleVector& x, const DoubleVector& y);

    void importData(QTextStream& out) const;
private:
    std::vector<std::shared_ptr<InterpolatorFun>> mShapes;

    void minimize(const Function& fun);

    void setWidth(double w);

    bool calcAmps(const DoubleVector& x, const DoubleVector& y);

    double mW;
    DoubleVector mUncertainties;
};

#endif // CURVEFITTING_H
