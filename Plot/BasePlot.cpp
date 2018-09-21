#include <algorithm>

#include "BasePlot.h"
#include "QCustomPlot/qcustomplot.h"

BasePlot::BasePlot(QObject *parent)
    : QObject (parent),
      m_plot(new QCustomPlot),
      m_toolBar(new QToolBar)
{
    setObjectName("BasePlot");
    m_actions.append(new QAction(QIcon(":/Icons/zoomIn"), "Zoom In"));
    m_actions.append(new QAction(QIcon(":/Icons/zoomOut"), "Zoom Out"));
    m_actions.append(new QAction(QIcon(":/Icons/openHand"), "Shift x-axis"));
    for(QAction * a : m_actions)
    {
        m_toolBar->addAction(a);
    }
    connect(m_plot, SIGNAL(beforeReplot()), this, SLOT(updateLimits()));
}

BasePlot::~BasePlot()
{

}

QCustomPlot *BasePlot::plot()
{
    return m_plot;
}

QToolBar *BasePlot::toolBar()
{
    return m_toolBar;
}

void BasePlot::updateLimits()
{
    int nGraphs = m_plot->graphCount();
    QCPRange xRange = m_plot->xAxis->range();
    QCPRange yRange;
    if(nGraphs != 0)
    {
        for(int i = 0; i < nGraphs; ++i)
        {
            QSharedPointer<QCPGraphDataContainer> data
                    = m_plot->graph(i)->data();
            xRange.lower = qMax(xRange.lower, data->begin()->key);
            xRange.upper = qMin(xRange.upper, std::prev(data->end())->key);
            QCPGraphDataContainer::const_iterator
                    firstIt = data->findBegin(xRange.lower),
                    lastIt = data->findEnd(xRange.upper);
            std::pair<QCPGraphDataContainer::const_iterator,
                    QCPGraphDataContainer::const_iterator> minMaxPair
                    = std::minmax(firstIt, lastIt,
                                  [](const QCPGraphData* a, const QCPGraphData* b)->bool
            {
                return a->value < b->value;
            });
            yRange.lower = minMaxPair.first->value;
            yRange.upper = minMaxPair.second->value;
        }
        m_plot->xAxis->setRange(xRange);
        m_plot->yAxis->setRange(yRange);
    }
}
