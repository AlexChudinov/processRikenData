#include "CompressedMS.h"

CompressedMS::CompressedMS(const Map& data, Interpolator::InterpType type)
    :
      m_pInterpolator(Interpolator::create(type, data))
{
}

CompressedMS::CompressedMS(Map &&data, IntegerInterpolator::InterpType type)
    :
      m_pInterpolator(Interpolator::create(type, data))
{
}

CompressedMS::~CompressedMS()
{
}
