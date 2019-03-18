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

void BasePlot::addGraph(const QPen &pen, GraphDescription desc)
{
    QCustomPlot::addGraph()->setPen(pen);
    mGraphProps.append(desc);
}

void BasePlot::onMouseRightClick()
{
    setSelectionRectMode(QCP::srmNone);
    setCursor(QCursor(Qt::ArrowCursor));
    replot();
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
        onMouseRightClick();
        Q_EMIT mouseRightClick();
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
                         SLOT(onZoomInAction()));
    m_toolBar->addAction(QIcon("://Icons//zoomOut"), "Zoom Out", this,
                         SLOT(onZoomOutAction()));
    m_toolBar->addAction(QIcon("://Icons//openHand"), "Shift x-axis", this,
                         SLOT(onDragAxisAction()));
    m_toolBar->addAction(QIcon("://Icons//image"), "Export image", this,
                         SLOT(onExportImageAction()));
}

void BasePlot::updateLimits()
{
    if(graphCount() != 0)
    {
        QCPRange xRange = xAxis->range();
        xAxis->rescale();
        QCPRange fullXRange = xAxis->range();
        xRange.lower = qMax(xRange.lower, fullXRange.lower);
        xRange.upper = qMin(xRange.upper, fullXRange.upper);
        xAxis->setRange(xRange);

        QCPRange yRange;
        QSharedPointer<QCPGraphDataContainer> data = graph(0)->data();
        QCPGraphDataContainer::const_iterator
                _First = data->findBegin(xRange.lower),
                _Last = data->findEnd(xRange.upper);
        std::pair
            < QCPGraphDataContainer::const_iterator,
              QCPGraphDataContainer::const_iterator > minMax =
                std::minmax_element
                (
                    _First, _Last,
                    []
                    (
                        const QCPGraphData& a,
                        const QCPGraphData& b
                    )->bool
                    {
                        return a.value < b.value;
                    }
                );
        yRange.lower = minMax.first->value;
        yRange.upper = minMax.second->value;
        for(int i = 1; i < graphCount(); ++i)
        {
            QSharedPointer<QCPGraphDataContainer> data = graph(i)->data();
            _First = data->findBegin(xRange.lower);
            _Last = data->findEnd(xRange.upper);
            std::pair
                < QCPGraphDataContainer::const_iterator,
                  QCPGraphDataContainer::const_iterator > minMax =
                    std::minmax_element
                    (
                        _First, _Last,
                        []
                        (
                            const QCPGraphData& a,
                            const QCPGraphData& b
                        )->bool
                        {
                            return a.value < b.value;
                        }
                    );
            yRange.lower = qMin(yRange.lower, minMax.first->value);
            yRange.upper = qMax(yRange.upper, minMax.second->value);
        }
        yAxis->setRange(yRange);
    }
}

void BasePlot::mouseMoveEvent(QMouseEvent *evt)
{
    QPoint pt = evt->pos();
    double
            x = xAxis->pixelToCoord(pt.x()),
            y = yAxis->pixelToCoord(pt.y());
    QCustomPlot::mouseMoveEvent(evt);
    Q_EMIT mouseCoordinateNotify
    (
        tr("x: %1 y: %2").arg(x).arg(y)
    );
}
