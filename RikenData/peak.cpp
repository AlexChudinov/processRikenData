#ifndef PEAK_CPP
#define PEAK_CPP

#include "peak.h"

double Peak::right() const
{
    return m_right;
}

void Peak::setRight(double right)
{
    m_right = right;
}

double Peak::center() const
{
    return m_center;
}

void Peak::setCenter(double center)
{
    m_center = center;
}

double Peak::height() const
{
    return m_height;
}

void Peak::setHeight(double height)
{
    m_height = height;
}

double Peak::left() const
{
    return m_left;
}

void Peak::setLeft(double left)
{
    m_left = left;
}

void Peak::parabolicMaximum
(
    double x1,
    double y0,
    double y1,
    double y2
)
{
    double c = y1;
    double b = .5 * (y2 - y0);
    double a = .5 * (y2 - 2.*y1 + y0);
    m_center = x1 - b / (2. * a);
    m_height = c - b * b / (4. * a);
}

#endif // PEAK_CPP
