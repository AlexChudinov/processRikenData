#include "CompressedMS.h"

CompressedMS::CompressedMS(const Map& data, const String& strInterpType)
    :
      m_pInterpolator(Interpolator::FactoryInstance::create(strInterpType.c_str(), data))
{
}

CompressedMS::CompressedMS(Map &&data, const String &strInterpType)
    :
      m_pInterpolator(Interpolator::FactoryInstance::create(strInterpType.c_str(), data))
{
}

CompressedMS::~CompressedMS()
{
}
