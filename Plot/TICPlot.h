#ifndef TICPLOT_H
#define TICPLOT_H
#include <QPointer>
#include <QMainWindow>

class BasePlot;
class QCPRange;

class TICPlot : public QMainWindow
{
    Q_OBJECT
public:
    using Uint = unsigned long long;

    TICPlot(QWidget * parent = Q_NULLPTR);

    Q_SIGNAL void cursorPosNotify(size_t);

    Q_SIGNAL void msLimitsNotify(size_t, size_t);

    Q_SLOT void updateLast(size_t msCount);

    Q_SLOT void updateLimits(Uint first, Uint last);

    Q_SLOT void plot();
private:
    QPointer<BasePlot> mPlot;
    Uint mFirstBin;
    Uint mLastBin;
    Uint mCursorPos;

    void setCursorPos(size_t cursorPos);

    /**
     * @brief setCursorLimits makes cursor height equal to screen
     */
    Q_SLOT void setCursorLimits(const QCPRange& range);

    Q_SLOT void onMouseClick(QMouseEvent * evt);

    void keyPressEvent(QKeyEvent * evt);

    /**
     * @brief onSelectMS switch mode to Mass Spec selection
     */
    Q_SLOT void onSelectMS();

    Q_SLOT void onMouseRelease(QMouseEvent * evt);

    /**
     * @brief checkLimits checks that mass spec indices are in the limits
     * If it is not then sets them to maximum/minimum allowed
     * @param xMin first index
     * @param xMax last index
     */
    void checkLimits(double& xMin, double& xMax) const;
};

#endif // TICPLOT_H
