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
    setWindowTitle("TIC");
    std::pair<Uint, Uint> minMaxTime = MyInit::instance()->massSpec()->blockingMinMaxTime();

    mFirstBin = minMaxTime.first;
    mLastBin = minMaxTime.second;

    mPlot->addGraph(QPen(Qt::blue, 3));
    mPlot->addGraph(QPen(Qt::red, 3), BasePlot::UpdateLimitsOff);
    setCentralWidget(mPlot.data());
    addToolBar(Qt::TopToolBarArea, mPlot->toolBar());

    mPlot->toolBar()->addAction(QIcon("://Icons//selectMS"),
                                "Select mass spectrum",
                                this,
                                &TICPlot::onSelectMS
                                );

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
        std::pair<Uint, Uint> minMaxTime = MyInit::instance()->massSpec()->blockingMinMaxTime();
        mFirstBin = qMin(mFirstBin, minMaxTime.first);
        mLastBin = qMax(mLastBin, minMaxTime.second);
        const MassSpec::MapUintUint ms = MyInit::instance()->massSpec()->blockingGetMassSpec(msCount - 1, msCount);
        double TIC = std::accumulate(ms.begin(), ms.end(), 0.0,
                                     [](double a, MassSpec::MapUintUint::const_reference b)->double
        {
            return a + b.second;
        });
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
    Q_ASSERT(last >= first);
    if(mFirstBin != first || mLastBin != last)
    {
        mFirstBin = first;
        mLastBin = last;
        plot();
    }
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

void TICPlot::onSelectMS()
{
    mPlot->setCursor(Qt::PointingHandCursor);
    mPlot->setSelectionRectMode(QCP::srmCustom);
}

void TICPlot::onMouseRelease(QMouseEvent *evt)
{
    if(evt->button() == Qt::LeftButton
            && cursor() == Qt::PointingHandCursor)
    {
        QCPSelectionRect * qcpRect
                = mPlot->selectionRect();
        QRect rect = qcpRect->rect();
        double xMin = mPlot->xAxis->pixelToCoord(rect.left());
        double xMax = mPlot->xAxis->pixelToCoord(rect.right());
        size_t First = static_cast<size_t>(std::round(xMin));
        size_t Last = static_cast<size_t>(std::round(xMax));
        Q_EMIT msLimitsNotify(First, Last);
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
        double fXPos = mPlot->xAxis->pixelToCoord(evt->pos().x());
        fXPos = fXPos > 0.0 ? fXPos : 0.0;
        size_t xPos = static_cast<size_t>(std::round(fXPos));
        xPos = xPos < MyInit::instance()->massSpec()->blockingSize() ?
                    xPos : MyInit::instance()->massSpec()->blockingSize() - 1;
        setCursorPos(xPos);
        mPlot->replot();
    }
}

void TICPlot::keyPressEvent(QKeyEvent *evt)
{
    if(evt->key() == Qt::Key_Left)
    {
        double fXPos = mPlot->graph(1)->data()->begin()->key;
        fXPos = fXPos < 1.0 ? fXPos : fXPos - 1;
        setCursorPos(static_cast<size_t>(fXPos));
        mPlot->replot();
    }
    else if(evt->key() == Qt::Key_Right)
    {
        double fXPos = mPlot->graph(1)->data()->begin()->key;
        fXPos = fXPos >= MyInit::instance()->massSpec()->blockingSize() ? fXPos : fXPos + 1;
        setCursorPos(static_cast<size_t>(fXPos));
        mPlot->replot();
    }
    else
    {
        QWidget::keyPressEvent(evt);
    }
}
