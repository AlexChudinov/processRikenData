#include "MSPlot.h"
#include "BasePlot.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"

MSPlot::MSPlot(QWidget *parent)
    :
      QMainWindow (parent),
      mPlot(new BasePlot(this))
{
    mPlot->addGraph();
    setCentralWidget(mPlot.data());
    addToolBar(Qt::TopToolBarArea, mPlot->toolBar());
    connect(MyInit::instance()->massSpec(), SIGNAL(massSpecsNumNotify(size_t)),
            this, SLOT(plotMS(size_t)));
    plotMS(MyInit::instance()->massSpec()->size());
}

void MSPlot::plotMS(size_t idx)
{
    if(idx != 0)
    {
        MassSpec * ms = MyInit::instance()->massSpec();
        const MassSpec::MapUintUint msData = ms->blockingGetMassSpec(idx-1, idx);

        if(!msData.empty())
        {
            QVector<double> vXData;
            QVector<double> vYData;
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
        }
        mPlot->replot();
        mPlot->rescaleAxes();
    }
}
