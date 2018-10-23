#include "TICPlot.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"
#include "Plot/BasePlot.h"

TICPlot::TICPlot(QWidget *parent)
    :
      QMainWindow (parent),
      mPlot(new BasePlot(this)),
      mFirstBin(0),
      mLastBin(0),
      mCursorPos(0)
{
    mPlot->addGraph(QPen(Qt::blue, 3));
    mPlot->addGraph(QPen(Qt::red, 3), BasePlot::UpdateLimitsOff);
    setCentralWidget(mPlot.data());
    addToolBar(Qt::TopToolBarArea, mPlot->toolBar());

    connect(mPlot.data(), SIGNAL(mousePress(QMouseEvent*)),
            this, SLOT(onMouseClick(QMouseEvent*)));

    connect(mPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
            this, SLOT(setCursorLimits(QCPRange)));
}

void TICPlot::updateLast(size_t msCount)
{
    if (msCount != 0)
    {
        blockSignals(true);
        std::pair<Uint, Uint> minMaxTime = MyInit::instance()->massSpec()->minMaxTime();
        mFirstBin = minMaxTime.first;
        mLastBin = minMaxTime.second;
        double TIC = MyInit::instance()->massSpec()->blockingLastTic(mFirstBin, mLastBin);
        size_t count = static_cast<size_t>(mPlot->graph(0)->data()->size());
        mPlot->graph(0)->addData(static_cast<double>(count), TIC);
        mPlot->rescaleAxes();
        setCursorPos(count);
        mPlot->replot();
        blockSignals(false);
    }
}

void TICPlot::updateLimits(Uint first, Uint last)
{
    std::pair<size_t, size_t> minMax = std::minmax(first, last);
    mFirstBin = minMax.first;
    mLastBin = minMax.second;
    plot();
}

void TICPlot::plot()
{
        MassSpec * ms = MyInit::instance()->massSpec();
        MassSpec::VectorUint ticData =
                ms->blockingGetIonCurrent(mFirstBin, mLastBin);
        if(!ticData.empty())
        {
            QVector<double> vXData, vYData;
            vXData.reserve(static_cast<int>(ticData.size()));
            vYData.reserve(vXData.size());
            for(size_t i = 0; i < ticData.size(); ++i)
            {
                vXData.push_back(i);
                vYData.push_back(ticData[i]);
            }
            mPlot->graph(0)->setData(vXData, vYData);
            mPlot->rescaleAxes();
            setCursorPos(ticData.size() - 1);
            mPlot->replot();
        }
}

void TICPlot::setCursorPos(size_t cursorPos)
{
    double xVal = static_cast<double>(cursorPos);
    double yVal1 = mPlot->yAxis->range().upper;
    double yVal0 = mPlot->yAxis->range().lower;
    mPlot->graph(1)->setData({xVal, xVal, xVal},{yVal0, yVal1, yVal0});
    Q_EMIT cursorPosNotify(cursorPos);
}

void TICPlot::setCursorLimits(const QCPRange& range)
{
    double yVal1 = range.upper;
    double yVal0 = range.lower;
    double xVal = mPlot->graph(1)->data()->at(0)->key;
    mPlot->graph(1)->setData({xVal, xVal, xVal},{yVal0, yVal1, yVal0});
}

void TICPlot::onMouseClick(QMouseEvent *evt)
{
    if(evt->button() == Qt::LeftButton && mPlot->cursor() == Qt::ArrowCursor)
    {
        double xPos = mPlot->xAxis->pixelToCoord(evt->pos().x());
        setCursorPos(static_cast<size_t>(std::round(xPos)));
        mPlot->replot();
    }
}
