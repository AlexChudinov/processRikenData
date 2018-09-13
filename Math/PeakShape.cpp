#include "PeakShape.h"


PeakShape::PeakShape(const CompressedMS& ms)
	:
	m_shape(ms)
{
	const Interpolator::Map& tab = ms.interp()->table();
	using Reference = Interpolator::Map::const_reference;
	using Iterator = Interpolator::Map::const_iterator;
	Iterator it = std::max_element
	(
		tab.cbegin(),
		tab.cend(),
		[](Reference rhs, Reference lhs)->bool
	{
		return rhs.second < lhs.second;
	}
	);

	if (it != tab.begin() && std::next(it) != tab.end())
	{
		m_peak.parabolicMaximum
		(
			it->first,
			std::prev(it)->second,
			it->second,
			std::next(it)->second
		);
	}
	else
	{
		m_peak.setCenter(it->first);
		m_peak.setHeight(it->second);
	}
}


PeakShape::~PeakShape()
{
}

PeakShape PeakShape::approximate(const CompressedMS& ms) const
{
	PeakShape peak(ms);
	CompressedMS tempMS(m_shape);
	
	tempMS.squeezeXScale
	(
		(peak.m_peak.center() - this->m_peak.center()) 
		/ this->m_peak.center()
	);

	tempMS.rescale();
	double s = tempMS.bestMatch
	(
		peak.m_shape,
		peak.m_shape.interp()->maxX(),
		peak.m_shape.interp()->table().size() / 2 - 1
	);
	tempMS.squeezeXScale(s);
	tempMS.rescale();

	double yFactor = 0.0, yFactor2 = 0.0;
	for (Interpolator::Map::const_reference e : tempMS.interp()->table())
	{
		yFactor += e.second * peak.shape().interp()->interpolate(e.first);
		yFactor2 += e.second * e.second;
	}
	tempMS.interp()->yFactor(yFactor/yFactor2);

	return PeakShape(tempMS);
}
