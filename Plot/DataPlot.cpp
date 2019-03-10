#include "DataPlot.h"
#include "BasePlot.h"

DataPlot::DataPlot
(
    const QVector<double>& x,
    const QVector<double>& y,
    QWidget *parent
)
    :
      QMainWindow(parent),
      mPlot(new BasePlot(this))
{
    mPlot->addGraph(QPen(Qt::blue, 3));
    mPlot->graph(0)->setData(x, y);
    setCentralWidget(mPlot);
    addToolBar(mPlot->toolBar());
}
