#include <algorithm>

#include "BasePlot.h"
#include "QCustomPlot/qcustomplot.h"

BasePlot::BasePlot(QObject *parent)
    : QObject (parent),
      m_plot(new QCustomPlot),
      m_toolBar(new QToolBar)
{
    setObjectName("BasePlot");

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

void BasePlot::onZoomInAction()
{
    m_plot->setSelectionRectMode(QCP::srmZoom);
    m_plot->setCursor(QCursor(QPixmap("://Icons//zoomIn")));
}

void BasePlot::onZoomOutAction()
{
    //Do not update limits when it is zoom out
    m_plot->blockSignals(true);

    m_plot->setSelectionRectMode(QCP::srmNone);
    m_plot->setCursor(QCursor(Qt::ArrowCursor));
    m_plot->rescaleAxes();
    m_plot->replot();

    m_plot->blockSignals(false);
}

void BasePlot::onDragAxisAction()
{
    m_plot->setCursor(QCursor(Qt::OpenHandCursor));
    m_plot->setSelectionRectMode(QCP::srmNone);
}

void BasePlot::onExportImageAction()
{
    QString fileFilt;
    QString fileName = QFileDialog::getSaveFileName
    (
        m_plot,
        "Save as an image",
        QString(),
        tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;PDF (*.pdf)"),
        &fileFilt
    );
    if(!fileName.isEmpty())
    {
        if(fileFilt == "PNG (*.png)") m_plot->savePng(fileName);
        if(fileFilt == "JPG (*.jpg)") m_plot->saveJpg(fileName);
        if(fileFilt == "BMP (*.bmp)") m_plot->saveBmp(fileName);
        if(fileFilt == "PDF (*.pdf)") m_plot->savePng(fileName);
    }
}

void BasePlot::onMouseClick(QMouseEvent * event)
{
    if(event->button() == Qt::RightButton)
    {
        m_plot->setSelectionRectMode(QCP::srmNone);
        m_plot->setCursor(QCursor(Qt::ArrowCursor));
    }
    else
    {
        if(m_plot->cursor().shape() == Qt::OpenHandCursor)
        {
            QPoint pt = event->pos();
            double x = m_plot->xAxis->pixelToCoord(pt.x());
            QCPRange range = m_plot->xAxis->range();
            double x0 = .5 * (range.upper + range.lower);
            if(x < x0)
                (range.lower *= 2) -= x;
            else
                (range.upper *= 2) -= x;
            m_plot->xAxis->setRange(range);
        }
    }
}

void BasePlot::createActions()
{
    m_toolBar->addAction(QIcon("://Icons//zoomIn"), "Zoom In", this,
                         &BasePlot::onZoomInAction);
    m_toolBar->addAction(QIcon("://Icons//zoomOut"), "Zoom Out", this,
                         &BasePlot::onZoomOutAction);
    m_toolBar->addAction(QIcon("://Icons//openHand"), "Shift x-axis", this,
                         &BasePlot::onDragAxisAction);
    m_toolBar->addAction(QIcon(":/Icons/image"), "Export image", this,
                         &BasePlot::onExportImageAction);
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
                    = std::minmax_element(firstIt, lastIt,
                                  [](const QCPGraphData& a, const QCPGraphData& b)->bool
            {
                return a.value < b.value;
            });
            yRange.lower = minMaxPair.first->value;
            yRange.upper = minMaxPair.second->value;
        }
        m_plot->xAxis->setRange(xRange);
        m_plot->yAxis->setRange(yRange);
    }
}
