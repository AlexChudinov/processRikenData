#include "Interpolator.h"
#include <algorithm>

Linear::Linear(const Map& tab) : m_table(tab)
{
    std::pair<Map::const_iterator, Map::const_iterator>
            minMax = std::minmax_element(tab.begin(), tab.end(),
    [](const Map::const_reference a, const Map::const_reference b)->bool
    {
        return a.second < b.second;
    });
    m_nMinY = minMax.first->second;
    m_nMaxY = minMax.second->second;
}

Linear::Linear(Map&& tab) : m_table(tab)
{
    std::pair<Map::const_iterator, Map::const_iterator>
            minMax = std::minmax_element(tab.begin(), tab.end(),
    [](const Map::const_reference a, const Map::const_reference b)->bool
    {
        return a.second < b.second;
    });
    m_nMinY = minMax.first->second;
    m_nMaxY = minMax.second->second;
}

Interpolator::InterpType Linear::type() const
{
    return Interpolator::LinearType;
}

double Linear::interpolate(double xVal) const
{
    xVal /= xFactor();
    if (xVal < minX() || xVal >= maxX()) return 0.0;
    Map::const_iterator it1 = m_table.upper_bound(xVal), it0 = std::prev(it1);
    double yVal = interpolate(it0, it1, xVal);
    return yVal;
}

Linear::String Linear::name() const
{
	return String("Linear");
}

double Linear::minX() const
{
    return double(m_table.begin()->first) * xFactor();
}

double Linear::maxX() const
{
    return double(m_table.rbegin()->first) * xFactor();
}

double Linear::minY() const
{
    return double(m_nMinY) * yFactor();
}

double Linear::maxY() const
{
    return double(m_nMaxY) * yFactor();
}

const Linear::Map & Linear::table() const
{
    return m_table;
}

Linear::Map &Linear::table()
{
    return m_table;
}

Interpolator::Pointer Interpolator::create(InterpType type, const Map &tab)
{
    switch(type)
    {
    case LinearType: return Pointer(new Linear(tab));
    default: return Pointer();
    }
}

Interpolator::Pointer Interpolator::create(InterpType type, Map &tab)
{
    switch(type)
    {
    case LinearType: return Pointer(new Linear(tab));
    default: return Pointer();
    }
}
