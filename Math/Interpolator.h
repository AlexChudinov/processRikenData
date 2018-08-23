#pragma once

#include "Factory.h"

/**
 * Interpolating procedures
 */
class Interpolator
{
public:
	using String = std::string;
	using Map = std::map<double, double>;
	DEF_ADD_FACTORY_DEFS(Interpolator, Map)

	virtual ~Interpolator(){}

	//Interpolate y-value using x-value
	virtual double interpolate(double xVal) const = 0;

	//Returns max and min x-values
	virtual double minX() const = 0;
	virtual double maxX() const = 0;

	//Returns name of a current instance
	virtual String name() const = 0;

	//Integrate function over the xMin xMax range
	virtual double integrate(double xMin, double xMax) const = 0;

	//Returns table of interpolating values
	virtual const Map& table() const = 0;
};

class Linear : public Interpolator
{
	Map m_table;

public:

	Linear(const Map& tab) : m_table(tab){}

	Linear(Map&& tab) : m_table(tab) {}

	class Constructor : public Interpolator::Constructor
	{
	public:
		virtual Pointer create(const Map& tab) const;
		virtual Pointer create(Map&& tab) const;
	};

	DEF_ADD_CONSTRUCTOR_INSTANCE

	virtual double interpolate(double xVal) const;

	virtual String name() const;
	
	virtual double minX() const;
	virtual double maxX() const;

	virtual double integrate(double xMin, double xMax) const;

	virtual const Map& table() const;
private:

	inline double interpolate(
		const Map::const_iterator& it0,
		const Map::const_iterator& it1,
		double xVal) const
	{
		return 
			(it1->second - it0->second) / (it1->first - it0->first) 
			* (xVal - it0->first) + it0->second;
	}
};

//Dot product of two interpolators, one with the shift scale
double dotProductScaleShift(const Interpolator& i1, const Interpolator& i2, double shift = 0.0);

//Max shift
double maxShift(const Interpolator& i1, const Interpolator& i2);

//Dot product of two interpolators
double dotProduct(const Interpolator& i1, const Interpolator& i2);