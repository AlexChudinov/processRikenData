#include "MSPlot.h"
#include "BasePlot.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"

MSPlot::MSPlot(QWidget *parent)
    :
      QMainWindow (parent),
      mPlot(new BasePlot(this)),
      mFirst(0),
      mLast(0)
{
    mPlot->addGraph();
    setCentralWidget(mPlot.data());
    addToolBar(Qt::TopToolBarArea, mPlot->toolBar());
    plotMS();
}

void MSPlot::plotMS()
{
    MassSpec * ms = MyInit::instance()->massSpec();
    const MassSpec::MapUintUint msData
            = ms->blockingGetMassSpec(mFirst, mLast);

    if(!msData.empty())
    {
        QVector<double> vXData, vYData;
        vXData.reserve(static_cast<int>(msData.size()) + 2);
        vYData.reserve(vXData.size());
        vXData.push_back(msData.begin()->first - 1);
        vYData.push_back(0.0);
        for(MassSpec::MapUintUint::const_reference r : msData)
        {
            if(*vXData.rbegin() + 1 < r.first)
            {
                if(r.first - 1 > *vXData.rbegin() + 1)
                {
                    vXData.push_back(*vXData.rbegin() + 1);
                    vYData.push_back(0.0);
                }
                vXData.push_back(r.first - 1);
                vYData.push_back(0.0);
            }
            vXData.push_back(r.first);
            vYData.push_back(r.second);
        }
        mPlot->graph(0)->setData(vXData, vYData);
        mPlot->replot();
        mPlot->rescaleAxes();
    }
}

void MSPlot::setLimits(size_t first, size_t last)
{
    mFirst = last == 0 ? mFirst = 0 : mFirst = first;
    mLast = last;
    plotMS();
}

void MSPlot::updateLast()
{
    MassSpec * ms = MyInit::instance()->massSpec();
    const size_t n = ms->blockingSize();
    if(n != 0)
    {
        setLimits(n-1, n);
    }
}
