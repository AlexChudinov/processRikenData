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
