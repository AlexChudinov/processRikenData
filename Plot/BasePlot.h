#ifndef BASEPLOT_H
#define BASEPLOT_H

#include <QObject>

class QCustomPlot;
class QToolBar;
class QAction;

class BasePlot : public QObject
{
    Q_OBJECT

public:

    explicit BasePlot(QObject *parent = nullptr);

    virtual ~BasePlot();

    QCustomPlot * plot();

    QToolBar * toolBar();

private:

    QCustomPlot * m_plot;

    QList<QAction*> m_actions;

    QToolBar * m_toolBar;

    /**
     * @brief controlLimits watch that QCustomPlot should never
     * run out of the boundaries
     */
    void controlLimits();
    Q_SLOT void updateLimits();
};

#endif // BASEPLOT_H
