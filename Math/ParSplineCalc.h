#ifndef PARSPLINECALC_H
#define PARSPLINECALC_H

#include <vector>

/**
 * @brief The ParSplineCalc class calculates spline in parallel using
 * optimized memory consumption. Error checking is minimal
 */
class ParSplineCalc
{
    friend class CompressedMS;
private:
    using VectorDouble = std::vector<double>;

    /**
     * @brief logSpline calculates smoothed y-values using log scale spline
     * yIn values will be accepted
     * @param yOut - array of smoothed data
     * @param yIn - array of not smoothed data
     * @param p - smoothness parameter
     */
    static void logSplinePoissonWeights
    (
        VectorDouble& yOut,
        const VectorDouble& yIn,
        double p
    );
};

#endif // PARSPLINECALC_H
