#ifndef COMPRESSEDMS_H
#define COMPRESSEDMS_H

#include "Math/Interpolator.h"
#include <vector>

/**
 * @brief The CompressedMS class defines mass spec with only none zero values stored
 */
class CompressedMS
{
public:
    using Interpolator = IntegerInterpolator;
    using Map = Interpolator::Map;
    using String = Interpolator::String;
    using uint64_t = unsigned long long;
    using uint32_t = unsigned int;
    using VectorInt = std::vector<uint32_t>;

    //Compress vector
    CompressedMS(const VectorInt& vVals, size_t tMin,
                 IntegerInterpolator::InterpType type = Interpolator::LinearType);

    CompressedMS(const Map& data, IntegerInterpolator::InterpType type = Interpolator::LinearType);
    CompressedMS(Map&& data, IntegerInterpolator::InterpType type = Interpolator::LinearType);
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
    uint64_t match(const CompressedMS& msRef) const;

    /**
     * @brief bestMatch looks for the shift value that matches best. It makes discrete search in the
     * units of supplied max ToF value. Routine keep it unshifted
     * @param msRef - reference mass spectrum
     * @param nMaxTime - max ToF value
     * @param ok - flag to check that computation is ok
     * @return best relative shift value
     */
    double bestMatch(const CompressedMS& msRef, int nMaxTime, bool * ok = nullptr) const;

    /**
     * @brief addToAcc adds the mass spectrum to accumulating one
     * @param msAcc accumulating mass spectrum
     */
    void addToAcc(CompressedMS &msAcc) const;

    /**
     * @brief rescale recalculates the scale of interpolator, so the xscale factor becomes 1.0
     */
    void rescale();
private:
    Interpolator::Pointer m_pInterpolator;
};

#endif // COMPRESSEDMS_H
