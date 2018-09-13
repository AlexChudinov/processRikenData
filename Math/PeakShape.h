#ifndef PEAK_SHAPE_H
#define PEAK_SHAPE_H

#include "../RikenData/CompressedMS.h"

class PeakShape
{
	//Peak shape holder
	CompressedMS m_shape;
	//Peak position at the time scale
	Peak m_peak;
public:
	PeakShape(const CompressedMS& ms);
	virtual ~PeakShape();

	//Approximates peak position and intensity
	PeakShape approximate(const CompressedMS& ms) const;

	const CompressedMS& shape() const { return m_shape; }

	const Peak& peak() const { return m_peak; }
};

#endif