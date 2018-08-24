#ifndef _INTERPOLATOR_
#define _INTERPOLATOR_

#include <map>
#include "Utils/Factory.h"

/**
 * Interpolating procedures
 */
class IntegerInterpolator
{
    /**
     * @brief m_fXFactor multyplicator for xscale to get float value
     */
    double m_fXFactor;
    /**
     * @brief m_fYFactor multiplycator for yscale to get float value
     */
    double m_fYFactor;

public:
	using String = std::string;
    using Map = std::map<int, int>;
    DEF_ADD_FACTORY_DEFS(IntegerInterpolator, Map)

    IntegerInterpolator() : m_fXFactor(1.0), m_fYFactor(1.0) {}
    virtual ~IntegerInterpolator(){}

	//Interpolate y-value using x-value
	virtual double interpolate(double xVal) const = 0;

	//Returns max and min x-values
    virtual int minX() const = 0;
    virtual int maxX() const = 0;

    //Returns max and min y-values
    virtual int minY() const = 0;
    virtual int maxY() const = 0;

	//Returns name of a current instance
	virtual String name() const = 0;

	//Returns table of interpolating values
	virtual const Map& table() const = 0;

    inline double xFactor() const { return m_fXFactor; }
    inline double yFactor() const { return m_fYFactor; }
    inline void xFactor(double fXFactor) { m_fXFactor = fXFactor; }
    inline void yFactor(double fYFactor) { m_fYFactor = fYFactor; }
};

class Linear : public IntegerInterpolator
{
    /**
     * @brief m_table ref values to proceed interpolation
     */
	Map m_table;

    /**
     * @brief m_nMinY keeps minimal y-value
     */
    int m_nMinY;
    /**
     * @brief m_nMaxY teeps maximal y-valuee
     */
    int m_nMaxY;
public:

    Linear(const Map& tab);

    Linear(Map&& tab);

    class Constructor : public IntegerInterpolator::Constructor
	{
	public:
		virtual Pointer create(const Map& tab) const;
		virtual Pointer create(Map&& tab) const;
	};

	DEF_ADD_CONSTRUCTOR_INSTANCE

	virtual double interpolate(double xVal) const;

	virtual String name() const;
	
    virtual int minX() const;
    virtual int maxX() const;

    virtual int minY() const;
    virtual int maxY() const;

	virtual const Map& table() const;
private:

    inline double interpolate
    (
        const Map::const_iterator& it0,
		const Map::const_iterator& it1,
        double xVal
    ) const
	{
        double fX0 = static_cast<double>(it0->first) * xFactor();
        double fX1 = static_cast<double>(it1->first) * xFactor();
        double fY0 = static_cast<double>(it0->second)* yFactor();
        double fY1 = static_cast<double>(it1->second)* yFactor();
		return (fY1 - fY0) / (fX1 - fX0) * (xVal - fX0) + fY0;
	}
};

#endif
