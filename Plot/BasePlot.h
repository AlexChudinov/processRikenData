#ifndef BASEPLOT_H
#define BASEPLOT_H

#include "QCustomPlot/qcustomplot.h"

class BasePlot : public QCustomPlot
{
    Q_OBJECT

public:

    explicit BasePlot(QWidget *parent = nullptr);

    virtual ~BasePlot();

    QToolBar * toolBar();

private:
    Q_SLOT void onZoomInAction();
    Q_SLOT void onZoomOutAction();
    Q_SLOT void onDragAxisAction();
    Q_SLOT void onExportImageAction();
    Q_SLOT void onMouseClick(QMouseEvent *event);

    void createActions();

    QPointer<QToolBar> m_toolBar;

    /**
     * @brief controlLimits watch that QCustomPlot should never
     * run out of the boundaries
     */
    void controlLimits();
    Q_SLOT void updateLimits();
};

#endif // BASEPLOT_H
