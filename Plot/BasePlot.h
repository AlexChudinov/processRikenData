#ifndef BASEPLOT_H
#define BASEPLOT_H

#include <QObject>

class QCustomPlot;
class QToolBar;
class QAction;
class QMouseEvent;

class BasePlot : public QObject
{
    Q_OBJECT

public:

    explicit BasePlot(QObject *parent = nullptr);

    virtual ~BasePlot();

    QCustomPlot * plot();

    QToolBar * toolBar();

private:
    Q_SLOT void onZoomInAction();
    Q_SLOT void onZoomOutAction();
    Q_SLOT void onDragAxisAction();
    Q_SLOT void onExportImageAction();
    Q_SLOT void onMouseClick(QMouseEvent *event);

    void createActions();

    QCustomPlot * m_plot;

    QToolBar * m_toolBar;

    /**
     * @brief controlLimits watch that QCustomPlot should never
     * run out of the boundaries
     */
    void controlLimits();
    Q_SLOT void updateLimits();
};

#endif // BASEPLOT_H
