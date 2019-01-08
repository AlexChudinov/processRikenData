#include "PlotPair.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"
#include <QInputDialog>

PlotPair::PlotPair(QWidget *parent) :
    QMainWindow(parent),
    mMsPlot(new BasePlot(this)),
    mTicPlot(new BasePlot(this))
{
    MassSpec * ms = MyInit::instance()->massSpec();
    connect(ms, SIGNAL(cleared()), this, SLOT(clearData()));
    connect(ms, SIGNAL(massSpecsNumNotify(size_t)),
            this, SLOT(massSpecNumsChanged(size_t)));

    connectPlots();

    createGraphs();

    QVBoxLayout * layout = new QVBoxLayout;
    QWidget * w = new QWidget;
    w->setLayout(layout);
    layout->setMargin(0);
    layout->addWidget(mMsPlot);
    layout->addWidget(mTicPlot);
    setCentralWidget(w);
    addToolBar(Qt::TopToolBarArea, mMsPlot->toolBar());

    connectActions();
}

void PlotPair::clearData()
{
    mMsPlot->graph(0)->data()->clear();
    mTicPlot->graph(0)->data()->clear();
}

void PlotPair::massSpecNumsChanged(size_t massSpecNum)
{
    MassSpec * ms = MyInit::instance()->massSpec();
    int curMsNum = mTicPlot->graph(0)->data()->size();
    for(;curMsNum < massSpecNum; ++curMsNum)
    {
        MassSpec::MapUintUint MS = ms->blockingGetMassSpec(curMsNum);
        double ticIdx = static_cast<double>(curMsNum);
        double tic = std::accumulate
        (
            MS.begin(),
            MS.end(),
            0.0,
            [](double a, MassSpec::MapUintUint::reference b)->double
        {
            return a + b.second;
        });
        QSharedPointer<QCPGraphDataContainer> msData(new QCPGraphDataContainer);
        for(MassSpec::MapUintUint::const_reference d : MS)
        {
            msData->add({d.first, d.second});
        }
        mMsPlot->graph(0)->setData(msData);
        mTicPlot->graph(0)->data()->add({ticIdx, tic});
        mMsPlot->rescaleAxes(); mMsPlot->replot();
        mTicPlot->rescaleAxes(); mTicPlot->replot();
    }
}

void PlotPair::onExportImage()
{
    QStringList items;
    items  << tr("MS") << tr("TIC");
    QString res = QInputDialog::getItem
    (
        this,
        tr("Export image"),
        tr("Choose graph"),
        items
    );
    if(res == "MS")
    {
        mMsPlot->onExportImageAction();
    }
    if(res == "TIC")
    {
        mTicPlot->onExportImageAction();
    }
}

void PlotPair::createGraphs()
{
    mMsPlot->addGraph(QPen(Qt::blue, 3));
    mTicPlot->addGraph(QPen(Qt::blue, 3));
    mTicPlot->addGraph(QPen(Qt::red, 3), BasePlot::UpdateLimitsOff);
}

void PlotPair::connectActions()
{
    QList<QAction*> actions = mMsPlot->toolBar()->actions();
    for(QAction * a : actions)
    {
        if(a->text() == "Zoom In")
            connect(a, SIGNAL(triggered()),
                    mTicPlot.data(), SLOT(onZoomInAction()));
        if(a->text() == "Zoom Out")
            connect(a, SIGNAL(triggered()),
                    mTicPlot.data(), SLOT(onZoomOutAction()));
        if(a->text() == "Shift x-axis")
            connect(a, SIGNAL(triggered()),
                    mTicPlot.data(), SLOT(onDragAxisAction()));
        if(a->text() == "Export image")
        {
            disconnect(a, SIGNAL(triggered()),
                       mMsPlot.data(), SLOT(onExportImageAction()));
            connect(a, SIGNAL(triggered()), this, SLOT(onExportImage()));
        }
    }
}

void PlotPair::connectPlots()
{
    connect(mMsPlot.data(), SIGNAL(mouseRightClick()),
            mTicPlot.data(), SLOT(onMouseRightClick()));

    connect(mTicPlot.data(), SIGNAL(mouseRightClick()),
            mMsPlot.data(), SLOT(onMouseRightClick()));
}
