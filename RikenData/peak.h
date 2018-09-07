#ifndef PEAK_H
#define PEAK_H

#include <set>

/**
 * @brief The Peak class defines peak, its position, height and width
 */
class Peak
{
    double m_left;
    double m_right;
    double m_center;
    double m_height;
public:

    using PeakCollection = std::set<Peak>;

    Peak(double center = 0.0, double left = 0.0, double right = 0.0, double height = 0.0)
        :
          m_left(left), m_right(right), m_center(center), m_height(height)
    {}

    double left() const;
    void setLeft(double left);

    double right() const;
    void setRight(double right);


    double center() const;
    void setCenter(double center);

    double height() const;
    void setHeight(double height);

    /**
     * @brief operator < is needed to store peaks in a set addressing them by
     * their positions
     * @param pr
     * @return
     */
    inline bool operator < (const Peak& pr) const { return m_center < pr.center(); }

    /**
     * @brief parabolicMaximum calculates maxima position and height
     * using three points
     */
    void parabolicMaximum
    (
        double x1,
        double y0,
        double y1,
        double y2
    );
};

#endif // PEAK_H
