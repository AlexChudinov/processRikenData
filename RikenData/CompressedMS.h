#ifndef COMPRESSEDMS_H
#define COMPRESSEDMS_H

#include "Math/Interpolator.h"

/**
 * @brief The CompressedMS class defines mass spec with only none zero values stored
 */
class CompressedMS
{
public:
    using Interpolator = IntegerInterpolator;
    using Map = Interpolator::Map;
    using String = Interpolator::String;

    CompressedMS(const Map& data, IntegerInterpolator::InterpType type = Interpolator::LinearType);
    CompressedMS(Map&& data, IntegerInterpolator::InterpType type = Interpolator::LinearType);

    ~CompressedMS();

    const Interpolator * interp() const { return m_pInterpolator.get(); }

private:
    Interpolator::Pointer m_pInterpolator;
};

#endif // COMPRESSEDMS_H
