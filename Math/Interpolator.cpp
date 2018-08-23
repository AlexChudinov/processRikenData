#include "stdafx.h"
#include "Interpolator.h"

DEF_PRODUCTS_LIST(Interpolator, Interpolator::Map)
{
	DEF_PRODUCTS_LIST_ENTRY(Linear)
};

double Linear::interpolate(double xVal) const
{
	Map::const_iterator it1 = m_table.lower_bound(xVal), it0;
	if (it1 == m_table.begin()) it1 = std::next(it0 = it1);
	else if (it1 == m_table.end()) it0 = std::prev(--it1);
	else it0 = std::prev(it1);
	return interpolate(it0, it1, xVal);
}

Interpolator::String Linear::name() const
{
	return String("Linear");
}

double Linear::minX() const
{
	return m_table.begin()->first;
}

double Linear::maxX() const
{
	return m_table.rbegin()->first;
}

double Linear::integrate(double xMin, double xMax) const
{
	std::pair<double, double> MinMax = std::minmax(xMin, xMax);
	xMin = MinMax.first;
	xMax = min(m_table.rbegin()->first, MinMax.second);

	if (xMin >= m_table.rbegin()->first) return 0.0;

	Map::const_iterator
		itFirst = m_table.upper_bound(xMin),
		itEnd = std::prev(m_table.lower_bound(xMax));

	double yMin = interpolate(xMin), yMax = interpolate(xMax);
	double sum = .5 * (itFirst->second + yMin) * (itFirst->first - xMin);
	for (auto it = itFirst; it != itEnd; ++it)
	{
		sum += .5 * (std::next(itFirst)->second + itFirst->second)
			* (std::next(itFirst)->first - itFirst->first);
	}
	sum += .5 * (yMax + itFirst->second) * (yMax - itFirst->first);

	return sum;
}

const Interpolator::Map & Linear::table() const
{
	return m_table;
}

Interpolator::Pointer Linear::Constructor::create(const Map & tab) const
{
	return Pointer(new Linear(tab));
}

Interpolator::Pointer Linear::Constructor::create(Map && tab) const
{
	return Pointer(new Linear(tab));
}

double dotProductScaleShift(const Interpolator & i1, const Interpolator & i2, double shift)
{
	if (shift > -1.0) 
	{
		double res = 0.0, d = 1. + shift;
		Interpolator::Map::const_iterator it = i1.table().begin();
		for (; it != std::prev(i1.table().end()); ++it)
		{
			if (it->second != 0.0 || std::next(it)->second != 0.0)
			{
				res += i1.integrate(it->first, std::next(it)->first) 
					* i2.integrate(it->first*d, std::next(it)->second*d);
			}
		}

		return res;
	}
	else return 0.0;
}

double maxShift(const Interpolator & i1, const Interpolator & i2)
{
	//Smallest shift size
	const static double s_shiftAccuracy = 0.01e-6;
	double leftMax = 0.0, rightMax = 0.0;
	double leftMaxValue = dotProductScaleShift(i1, i2, -s_shiftAccuracy/2.),
		rightMaxValue = dotProductScaleShift(i1, i2, s_shiftAccuracy / 2.),
		maxValue = dotProductScaleShift(i1, i2);
	double shiftValue = 0.0;

	return shiftValue;
}

double dotProduct(const Interpolator & i1, const Interpolator & i2)
{
	return dotProductScaleShift(i1, i2);
}
