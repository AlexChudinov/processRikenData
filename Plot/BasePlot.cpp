#include <algorithm>
#include "BasePlot.h"

BasePlot::BasePlot(QWidget *parent)
    : QCustomPlot(parent),
      m_toolBar(new QToolBar)
{
    setObjectName("BasePlot");
    connect(this, SIGNAL(beforeReplot()),
            this, SLOT(updateLimits()));
    createActions();
    connect(this, SIGNAL(mousePress(QMouseEvent*)),
            this, SLOT(onMouseClick(QMouseEvent*)));
}

BasePlot::~BasePlot()
{

}

QToolBar *BasePlot::toolBar()
{
    return m_toolBar;
}

void BasePlot::onZoomInAction()
{
    setSelectionRectMode(QCP::srmZoom);
    setCursor(QCursor(QPixmap("://Icons//zoomIn")));
}

void BasePlot::onZoomOutAction()
{
    //Do not update limits when it is zoom out
    blockSignals(true);

    setSelectionRectMode(QCP::srmNone);
    setCursor(QCursor(Qt::ArrowCursor));
    rescaleAxes();
    replot();

    blockSignals(false);
}

void BasePlot::onDragAxisAction()
{
    setCursor(QCursor(Qt::OpenHandCursor));
    setSelectionRectMode(QCP::srmNone);
}

void BasePlot::onExportImageAction()
{
    QString fileFilt;
    QString fileName = QFileDialog::getSaveFileName
    (
        this,
        "Save as an image",
        QString(),
        tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;PDF (*.pdf)"),
        &fileFilt
    );
    if(!fileName.isEmpty())
    {
        if(fileFilt == "PNG (*.png)") savePng(fileName);
        if(fileFilt == "JPG (*.jpg)") saveJpg(fileName);
        if(fileFilt == "BMP (*.bmp)") saveBmp(fileName);
        if(fileFilt == "PDF (*.pdf)") savePng(fileName);
    }
}

void BasePlot::onMouseClick(QMouseEvent * event)
{
    if(event->button() == Qt::RightButton)
    {
        setSelectionRectMode(QCP::srmNone);
        setCursor(QCursor(Qt::ArrowCursor));
    }
    else
    {
        if(cursor().shape() == Qt::OpenHandCursor)
        {
            QPoint pt = event->pos();
            double x = xAxis->pixelToCoord(pt.x());
            QCPRange range = xAxis->range();
            double x0 = .5 * (range.upper + range.lower);
            if(x < x0)
                (range.lower *= 2) -= x;
            else
                (range.upper *= 2) -= x;
            xAxis->setRange(range);
            replot();
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
    int nGraphs = graphCount();
    QCPRange xRange = xAxis->range();
    QCPRange yRange;
    if(nGraphs != 0)
    {
        for(int i = 0; i < nGraphs; ++i)
        {
            QSharedPointer<QCPGraphDataContainer> data
                    = graph(i)->data();
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
        xAxis->setRange(xRange);
        yAxis->setRange(yRange);
    }
}
