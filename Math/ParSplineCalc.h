#ifndef PARSPLINECALC_H
#define PARSPLINECALC_H

#include <vector>
#include <QtConcurrent>

/**
 * @brief The ParSplineCalc class calculates spline in parallel using
 * optimized memory consumption. Error checking is minimal
 */
class ParSplineCalc
{
private:
    friend class CompressedMS;
    friend class InstanceLocker;

    ParSplineCalc(){}

    Q_DISABLE_COPY(ParSplineCalc)

    /**
     * @brief The InstanceLocker class locks and unlocks ParSplineCalc instance
     * automatically
     */
    class InstanceLocker
    {
        ParSplineCalc * m_instance;
    public:
        InstanceLocker(ParSplineCalc * instance)
            : m_instance(instance)
        {}

        ~InstanceLocker()
        {
            m_instance->clear();
            m_instance->freeInstance();
        }

        ParSplineCalc * operator -> () { return m_instance; }
    };

    static ParSplineCalc s_instance;

    //Only one thread can work with
    //the instance
    static QMutex s_mutex;

    using VectorDouble = std::vector<double>;

    //Vectors to store matrix values
    VectorDouble a, b, c, d, e, bb, r;

    /**
     * @brief getInstance locks mutex
     * @return
     */
    static InstanceLocker lockInstance();

    /**
     * @brief freeInstance releases mutex
     */
    static void freeInstance();

    /**
     * @brief clear frees allocated memory
     */
    void clear();

    /**
     * @brief logSpline calculates smoothed y-values using log scale spline
     * yIn values will be accepted
     * @param yOut - array of smoothed data
     * @param yIn - array of not smoothed data
     * @param p - smoothness parameter
     */
    void logSplinePoissonWeights
    (
        VectorDouble& yOut,
        const VectorDouble& yIn,
        double p
    );
};

#endif // PARSPLINECALC_H
