#include "MSPlot.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"

MSPlot::MSPlot(QWidget *parent)
    :
      QMainWindow (parent),
      mPlot(new BasePlot(this)),
      mFirst(0),
      mLast(0)
{
    mPlot->addGraph(QPen(Qt::blue, 3));
    setCentralWidget(mPlot.data());
    addToolBar(Qt::TopToolBarArea, mPlot->toolBar());
    plotMS();
}

void MSPlot::plotMS()
{
    if(mFirst != mLast)
    {
        MassSpec * ms = MyInit::instance()->massSpec();
        const MassSpec::MapUintUint msData
                = ms->blockingGetMassSpec(mFirst, mLast);
        plotMassSpec(msData);
    }
}

void MSPlot::setLimits(size_t first, size_t last)
{
    mFirst = last == 0 ? mFirst = 0 : mFirst = first;
    mLast = last;
    plotMS();
}

void MSPlot::updateLast(size_t msCount)
{
    if(msCount != 0)
    {
        mFirst = msCount - 1;
        mLast = msCount;
        plotMassSpec(MyInit::instance()->massSpec()->blockingGetMassSpec(msCount-1, msCount));
    }
}

void MSPlot::showMassSpec(size_t num)
{
    if(num != 0 && num < static_cast<size_t>(-1))
    {
        mFirst = num;
        mLast = num + 1;
        plotMS();
    }
}
