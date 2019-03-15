#include <QDialog>
#include <exception>

#include "../QMapPropsDialog.h"

#include "Base/BaseObject.h"
#include "DataPlot.h"
#include "BasePlot.h"
#include "Math/LogSplinePoissonWeight.h"

const QList<Qt::GlobalColor> DataPlot::s_colors
{
    Qt::black,
    Qt::darkGray,
    Qt::red,
    Qt::green,
    Qt::blue,
    Qt::cyan,
    Qt::magenta,
    Qt::darkRed,
    Qt::darkGreen,
    Qt::darkBlue,
    Qt::darkCyan,
    Qt::darkMagenta,
    Qt::darkYellow
};

DataPlot::DataPlot
(
    const QVector<double>& x,
    const QVector<double>& y,
    const QString& capture,
    const QString &xLabel,
    const QString &yLabel,
    QWidget *parent
)
    :
      QMainWindow(parent),
      mPlot(new BasePlot(this)),
      mData(new PlotData),
      mInterp(Interpolator::create("Linear"))
{
    mPlot->setWindowTitle(capture);
    setCentralWidget(mPlot);
    mPlot->toolBar()->addAction(QIcon("://Icons//splineSmoothing"), "Smooth data with spline", this,
                                SLOT(calculateSmoothing()));
    mPlot->toolBar()->addAction(QIcon("://Icons//interp"), "Choose interpolator", this,
                                SLOT(chooseInterpolator()));
    addToolBar(mPlot->toolBar());

    mPlot->xAxis->setLabel(xLabel);
    if(!yLabel.isEmpty()) mPlot->xAxis->setLabel(yLabel);

    mData->addData("Raw data", x, y);

    onShowMs();

    mPlot->rescaleAxes();
    mPlot->replot();
}

void DataPlot::calculateSmoothing()
{
     //Note 0 idx is always raw data
    try {
        const QCPRange range = mPlot->xAxis->range();
        const PlotData::DataVector& x0 = mData->begin()->first;
        const PlotData::DataVector& y0 = mData->begin()->second;
        PlotData::DataVector::const_iterator _First
                = std::prev(std::lower_bound(x0.begin(), x0.end(), range.lower));
        PlotData::DataVector::const_iterator _Last
                = std::upper_bound(x0.begin(), x0.end(), range.upper);
        const size_t n = static_cast<size_t>(std::distance(_First, _Last));
        PlotData::DataVector::const_iterator _First1 = y0.begin();
        std::advance(_First1, std::distance(x0.begin(), _First));
        Interpolator::Vector x(n), y(n);
        for(size_t i = 0; _First != _Last; ++_First, ++_First1, ++i)
        {
            x[i] = *_First;
            y[i] = *_First1;
        }

        double step;
        Interpolator::Vector yNew = mInterp->equalStepData(x, y, step),
                xNew(yNew.size()),
                ySmoothed;

        double xx0 = x[0] - step;
        for(double & xx : xNew) xx = xx0 += step;

        std::replace_if
        (
            yNew.begin(),
            yNew.end(),
            [](double val)->bool { return val < 0; },
            0.
        );

        createSmoother();
        mSmoother->run(ySmoothed, yNew);

        mData->addData
        (
            tr("Smoothed data mz: %1 - %2").arg(x0.first()).arg(x0.last()),
             QVector<double>::fromStdVector(xNew),
             QVector<double>::fromStdVector(ySmoothed)
        );

        onShowMs();
    }
    catch (const std::exception& e)
    {
        QString except = tr("An exception \"") + e.what() + "\" catched!";
        QMessageBox::warning(this, "Data plot message", except);
    }

}

void DataPlot::createSmoother()
{
    QString item = QInputDialog::getItem
    (
        this,
        tr("Smoother type"),
        tr("Choose smoother type: "),
        Smoother::types()
    );
    mSmoother.reset(Smoother::create(item).release());

    QMapPropsDialog dialog(this);
    dialog.setProps(mSmoother->params());
    dialog.exec();
    mSmoother->setParams(dialog.props());
}

void DataPlot::chooseInterpolator()
{
    QString item = QInputDialog::getItem
    (
        this,
        tr("Set interpolator"),
        tr("Interpolator type"),
        Interpolator::names()
    );
    mInterp.reset(Interpolator::create(item).release());
}

void DataPlot::onShowMs()
{
    int cnt = 0;
    mPlot->clearGraphs();
    for(const auto& d : *mData)
    {
        Qt::GlobalColor color = s_colors[cnt % s_colors.size()];
        mPlot->addGraph(QPen(color, 3));
        QCPGraph * g = mPlot->graph(cnt++);
        g->setData(d.first, d.second);
    }
    mPlot->replot();
}

void DataPlot::on_showPlotsTriggered()
{
    PropertiesListForm dialog;
}

PropertiesOfPlot::PropertiesOfPlot(QWidget *parent)
    :
      PropertiesListForm(parent)
{

}

PropertiesOfPlot::~PropertiesOfPlot()
{

}
