#ifndef PEAK_UNCERTAINTY_H
#define PEAK_UNCERTAINTY_H

#include "MathObject.h"
#include <memory>

class Smoother;
class CompressedMS;

//Peak uncertainty calculation strategy
class PeakUncertainty : public MathObject
{
public:
	PeakUncertainty();
	virtual ~PeakUncertainty();

	virtual void run
	(
		CompressedMS * msOut,
		Smoother * s,
		const CompressedMS * msIn
	) = 0;
};

#endif