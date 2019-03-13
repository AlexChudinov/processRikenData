#include <QDialog>
#include <exception>

#include "../QMapPropsDialog.h"

#include "DataPlot.h"
#include "BasePlot.h"
#include "Math/LogSplinePoissonWeight.h"

DataPlot::DataPlot
(
    const QVector<double>& x,
    const QVector<double>& y,
    const QString& capture,
    QWidget *parent
)
    :
      QMainWindow(parent),
      mPlot(new BasePlot(this)),
      mInterp(Interpolator::create("Linear"))
{
    mPlot->addGraph(QPen(Qt::blue, 3));
    mPlot->graph(0)->setData(x, y);
    mPlot->setWindowTitle(capture);
    setCentralWidget(mPlot);
    mPlot->toolBar()->addAction(QIcon("://Icons//splineSmoothing"), "Smooth data with spline", this,
                                SLOT(calculateSmoothing()));
    mPlot->toolBar()->addAction(QIcon("://Icons//interp"), "Choose interpolator", this,
                                SLOT(chooseInterpolator()));
    addToolBar(mPlot->toolBar());
}

void DataPlot::calculateSmoothing()
{
    try {
        QCPGraph * g;
        if(mPlot->graphCount() != 2)
        {
            mPlot->addGraph(QPen(Qt::red, 3));
        }
        g = mPlot->graph(1);
        QSharedPointer<QCPGraphDataContainer> data = mPlot->graph(0)->data();
        const QCPRange range = mPlot->xAxis->range();
        QCPGraphDataContainer::const_iterator _First = data->findBegin(range.lower);
        QCPGraphDataContainer::const_iterator _Last = data->findEnd(range.upper);
        const size_t n = static_cast<size_t>(std::distance(_First, _Last));
        Interpolator::Vector x(n), y(n);
        for(size_t i = 0; _First != _Last; ++_First, ++i)
        {
            x[i] = _First->key;
            y[i] = _First->value;
        }

        double step;
        Interpolator::Vector yNew = mInterp->equalStepData(x, y, step),
                xNew(yNew.size()),
                ySmoothed;

        double x0 = x[0] - step;
        for(double & xx : xNew) xx = x0 += step;

        std::replace_if
        (
            yNew.begin(),
            yNew.end(),
            [](double val)->bool { return val < 0; },
            0.
        );

        createSmoother();
        mSmoother->run(ySmoothed, yNew);

        g->setData(QVector<double>::fromStdVector(xNew), QVector<double>::fromStdVector(ySmoothed));
        mPlot->replot();
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
