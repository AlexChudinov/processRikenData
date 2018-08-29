#ifndef COMPRESSEDMS_H
#define COMPRESSEDMS_H

#include "Math/Interpolator.h"
#include <set>
#include <vector>

/**
 * @brief The Peak class defines peak, its position, height and width
 */
class Peak
{
    double m_left;
    double m_right;
    double m_center;
    double m_height;
public:

    using PeakCollection = std::set<Peak>;

    Peak(double center, double left = 0.0, double right = 0.0, double height = 0.0)
        :
          m_left(left), m_right(right), m_center(center), m_height(height)
    {}

    double left() const;
    void setLeft(double left);

    double right() const;
    void setRight(double right);


    double center() const;
    void setCenter(double center);

    double height() const;
    void setHeight(double height);

    inline bool operator < (const Peak& pr) const { return m_center < pr.center(); }
};

/**
 * @brief The CompressedMS class defines mass spec with only none zero values stored
 */
class CompressedMS
{
public:
    using Map = Interpolator::Map;
    using String = Interpolator::String;
    using uint64_t = unsigned long long;
    using uint32_t = unsigned int;
    using VectorInt = std::vector<uint32_t>;
    using VectorDouble = std::vector<double>;

    //Compress vector
    CompressedMS(const VectorInt& vVals, size_t tMin,
                 Interpolator::InterpType type = Interpolator::LinearType);

    CompressedMS(const Map& data, Interpolator::InterpType type = Interpolator::LinearType);
    CompressedMS(Map&& data, Interpolator::InterpType type = Interpolator::LinearType);
    //Impement copy constructors because pointer for Interpolator was used
    CompressedMS(const CompressedMS& ms);
    CompressedMS(CompressedMS&& ms);
    CompressedMS& operator=(const CompressedMS& ms);
    CompressedMS& operator=(CompressedMS&& ms);

    ~CompressedMS();

    const Interpolator * interp() const { return m_pInterpolator.get(); }
    Interpolator * interp() { return m_pInterpolator.get(); }

    /**
     * @brief squeezeXScale squeeze current mass spectrum scale multyplying x-values by (1 + s)
     * if s < -1 or s > 1 then do nothing
     * @param s
     */
    void squeezeXScale(double s);

    /**
     * @brief match matches given mass spectrum with the reference one
     * @param msRef - reference mass spectrum
     * @return inner product
     */
    double match(const CompressedMS& msRef) const;

    /**
     * @brief bestMatch looks for the shift value that matches best. It makes discrete search in the
     * units of supplied max ToF value. Routine keep it unshifted
     * @param msRef - reference mass spectrum
     * @param nMaxTime - max ToF value
     * @param ok - flag to check that computation is ok
     * @return best relative shift value
     */
    double bestMatch(const CompressedMS& msRef, size_t nMaxTime, size_t nTimeInterval = 300) const;

    /**
     * @brief addToAcc adds the mass spectrum to accumulating one
     * @param msAcc accumulating mass spectrum
     */
    void addToAcc(CompressedMS &msAcc) const;

    /**
     * @brief rescale recalculates the scale of interpolator, so the xscale factor becomes 1.0
     */
    void rescale();

    /**
     * @brief logSplineSmoothing applies spline smoothing to mass spectrum
     * data
     * @param p - smoothness param
     */
    void logSplineSmoothing(double p);
    void logSplineParamLessSmoothing();

    /**
     * @brief getPeaks calculates peaks with parameters using logarithmic spline smoothing
     * @param p - spline smoothness param
     * @return
     */
    Peak::PeakCollection getPeaks(double p) const;

    /**
     * @brief sumSqDev estimates sum of squares of deviations between two mass spectra
     * @param ms1
     * @return
     */
    CompressedMS::uint64_t sumSqDev(const CompressedMS& ms) const;

    /**
     * @brief totalIonCount calculates the total number of ion counts for the mass spec
     * @return
     */
    CompressedMS::uint64_t totalIonCount() const;
private:
    Interpolator::Pointer m_pInterpolator;

    /**
     * @brief transformToVector transform mass spec data to full scale
     * mass spectrum should be rescaled
     * @return vector of intensities
     */
    VectorDouble transformToVector() const;
};

#endif // COMPRESSEDMS_H
