#ifndef BASEPLOT_H
#define BASEPLOT_H

#include "QCustomPlot/qcustomplot.h"

class BasePlot : public QCustomPlot
{
    Q_OBJECT

public:

    friend class PlotPair;

    enum GraphDescription
    {
        UpdateLimitsOn,
        UpdateLimitsOff
    };

    explicit BasePlot(QWidget *parent = nullptr);

    virtual ~BasePlot();

    QToolBar * toolBar();

    void addGraph(const QPen& pen, GraphDescription desc = UpdateLimitsOn);

    Q_SIGNAL void mouseRightClick();

    Q_SIGNAL void mouseCoordinateNotify(QString);

    Q_SLOT void onMouseRightClick();

private:
    Q_SLOT void onZoomInAction();
    Q_SLOT void onZoomOutAction();
    Q_SLOT void onDragAxisAction();
    Q_SLOT void onExportImageAction();
    Q_SLOT void onMouseClick(QMouseEvent *event);

    void createActions();

    QPointer<QToolBar> m_toolBar;
    QList<GraphDescription> mGraphProps;

    /**
     * @brief controlLimits watch that QCustomPlot should never
     * run out of the boundaries
     */
    void controlLimits();
    Q_SLOT void updateLimits();

    void mouseMoveEvent(QMouseEvent * evt);
};

#endif // BASEPLOT_H
