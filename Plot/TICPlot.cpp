#include "TICPlot.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"
#include "Plot/BasePlot.h"

TICPlot::TICPlot(QWidget *parent)
    :
      QMainWindow (parent),
      mFirst(0),
      mLast(0)
{
    mPlot->addGraph();
    mPlot->addGraph();
    setCentralWidget(mPlot.data());
    addToolBar(Qt::TopToolBarArea, mPlot->toolBar());
}

void TICPlot::updateLast()
{

}

void TICPlot::updateLimits(size_t first, size_t last)
{
    mFirst = first;
    mLast = last;
    plot();
}

void TICPlot::plot()
{
    if (mFirst != mLast)
    {
        MassSpec * ms = MyInit::instance()->massSpec();
        MassSpec::MapUintUint ticData =
                ms->blockingGetIonCurrent(mFirst, mLast);
        if(!ticData.empty())
        {
            QVector<double> vXData, vYData;
            vXData.reserve(static_cast<int>(ticData.size()));
            vYData.reserve(vXData.size());
            for(MassSpec::MapUintUint::const_reference r : ticData)
            {
                vXData.push_back(r.first);
                vYData.push_back(r.second);
            }
            mPlot->graph(0)->setData(vXData, vYData);
            mPlot->replot();
            mPlot->rescaleAxes();
        }
    }
}
