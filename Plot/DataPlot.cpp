#include "DataPlot.h"
#include "BasePlot.h"

DataPlot::DataPlot
(
    const QVector<double>& x,
    const QVector<double>& y,
    const QString& capture,
    QWidget *parent
)
    :
      QMainWindow(parent),
      mPlot(new BasePlot(this))
{
    mPlot->addGraph(QPen(Qt::blue, 3));
    mPlot->graph(0)->setData(x, y);
    mPlot->setWindowTitle(capture);
    setCentralWidget(mPlot);
    addToolBar(mPlot->toolBar());
}

void DataPlot::calculateSmoothing()
{
    QCPGraph * g;
    if(mPlot->graphCount() != 2)
    {
        mPlot->addGraph(QPen(Qt::red, 3));
    }
    g = mPlot->graph(1);
    QSharedPointer<QCPGraphDataContainer> data = mPlot->graph(0)->data();
    const QCPRange range = mPlot->xAxis->range();
    QCPGraphDataContainer::const_iterator _First = data->findBegin(range.lower);
    QCPGraphDataContainer::const_iterator _Last = data->findEnd(range.upper);
}
