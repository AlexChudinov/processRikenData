#include "MSPlot.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"

MSPlot::MSPlot(QWidget *parent)
    :
      QMainWindow (parent),
      mPlot(new BasePlot(this))
{
    setLimits(0, 0);
    mPlot->addGraph(QPen(Qt::blue, 3));
    setCentralWidget(mPlot.data());
    addToolBar(Qt::TopToolBarArea, mPlot->toolBar());
    plot();
}

void MSPlot::plot()
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
    if(last == 0)
    {
        mFirst = last;
    }
    else
    {
        mFirst = first;
    }
    mLast = last;
    setWindowTitle(QString("MS: %1 - %2").arg(mFirst).arg(mLast));
    plot();
}

void MSPlot::updateLast(size_t msCount)
{
    if(msCount != 0)
    {
        setLimits(msCount - 1, msCount);
        plot();
    }
}

void MSPlot::showMassSpec(size_t num)
{
    if(num != 0 && num < static_cast<size_t>(-1))
    {
        setLimits(num, num + 1);
        plot();
    }
}
