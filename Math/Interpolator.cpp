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

double Linear::interpolate(double xVal) const
{
    int nXVal = static_cast<int>(xVal / xFactor());
    if (nXVal < minX() || nXVal >= maxX()) return 0.0;
    Map::const_iterator it1 = m_table.upper_bound(nXVal), it0 = std::prev(it1);
	return interpolate(it0, it1, xVal);
}

Linear::String Linear::name() const
{
	return String("Linear");
}

int Linear::minX() const
{
    return double(m_table.begin()->first) * xFactor();
}

int Linear::maxX() const
{
    return double(m_table.rbegin()->first) * xFactor();
}

int Linear::minY() const
{
    return double(m_nMinY) * yFactor();
}

int Linear::maxY() const
{
    return double(m_nMaxY) * yFactor();
}

const Linear::Map & Linear::table() const
{
	return m_table;
}

IntegerInterpolator::Pointer IntegerInterpolator::create(InterpType type, const Map &tab)
{
    switch(type)
    {
    case LinearType: return Pointer(new Linear(tab));
    default: return Pointer();
    }
}

IntegerInterpolator::Pointer IntegerInterpolator::create(InterpType type, Map &tab)
{
    switch(type)
    {
    case LinearType: return Pointer(new Linear(tab));
    default: return Pointer();
    }
}
