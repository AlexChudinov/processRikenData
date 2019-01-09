#include "PlotPair.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"
#include <QInputDialog>

PlotPair::PlotPair(QWidget *parent) :
    QMainWindow(parent),
    mMsPlot(new BasePlot(this)),
    mTicPlot(new BasePlot(this))
{
    createGraphs();

    MassSpec * ms = MyInit::instance()->massSpec();
    connect(ms, SIGNAL(cleared()), this, SLOT(clearData()));
    massSpecNumsChanged
    (
        ms->blockingSize()
    );
    connect(ms, SIGNAL(massSpecsNumNotify(size_t)),
            this, SLOT(massSpecNumsChanged(size_t)));

    connectPlots();

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

void PlotPair::setTicCursorPos(double x)
{
    x = std::round(x);
    if(x < 0.0) x = 0.0;
    MassSpec * ms = MyInit::instance()->massSpec();
    if(x >= ms->blockingSize()) x = ms->blockingSize() - 1;
    size_t idx = static_cast<size_t>(x);
    MassSpec::MapUintUint MS = ms->blockingGetMassSpec(idx);
    QSharedPointer<QCPGraphDataContainer> msData(new QCPGraphDataContainer);
    for(MassSpec::MapUintUint::const_reference d : MS)
    {
        msData->add
        (
            {
                static_cast<double>(d.first),
                static_cast<double>(d.second)
            }
        );
    }
    mMsPlot->graph(0)->setData(msData);
    mMsPlot->rescaleAxes();
    mMsPlot->replot();

    if(idx < static_cast<int>(mTicPlot->graph(0)->data()->size()))
    {
        double ymax = std::lower_bound
        (
            mTicPlot->graph(0)->data()->constBegin(),
            mTicPlot->graph(0)->data()->constEnd(),
            QCPGraphData::fromSortKey(x),
            qcpLessThanSortKey<QCPGraphData>
        )->value;
        mTicPlot->graph(1)->data()->set({{x,0}, {x, ymax}});
    }
    else
    {
        int curMsNum = mTicPlot->graph(0)->data()->size();
        double tic, ticIdx;
        for(;curMsNum <= idx; ++curMsNum)
        {
            MassSpec::MapUintUint MS = ms->blockingGetMassSpec(curMsNum);
            ticIdx = static_cast<double>(curMsNum);
            tic = std::accumulate
            (
                MS.begin(),
                MS.end(),
                0.0,
                [](double a, MassSpec::MapUintUint::reference b)->double
            {
                return a + b.second;
            });
            mTicPlot->graph(0)->data()->add({ticIdx, tic});
        }
        mTicPlot->graph(1)->data()->set({{ticIdx,0}, {ticIdx, tic}});
        mTicPlot->rescaleAxes();
    }
    mTicPlot->replot();
}

void PlotPair::clearData()
{
    mMsPlot->graph(0)->data()->clear();
    mTicPlot->graph(0)->data()->clear();
    mTicPlot->graph(1)->data()->clear();
}

void PlotPair::massSpecNumsChanged(size_t massSpecNum)
{
    if(massSpecNum != 0)
    {
        setTicCursorPos(static_cast<double>(massSpecNum) - 1.);
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

void PlotPair::keyPressEvent(QKeyEvent *evt)
{
    if(evt->key() == Qt::Key_Left)
    {
        double fXPos = mTicPlot->graph(1)->data()->begin()->key;
        setTicCursorPos(static_cast<size_t>(--fXPos));
    }
    else if(evt->key() == Qt::Key_Right)
    {
        double fXPos = mTicPlot->graph(1)->data()->begin()->key;
        setTicCursorPos(static_cast<size_t>(++fXPos));
    }
    else
    {
        QWidget::keyPressEvent(evt);
    }
}

void PlotPair::createGraphs()
{
    mMsPlot->addGraph(QPen(Qt::blue, 3));
    mTicPlot->addGraph(QPen(Qt::blue, 3));
    mTicPlot->addGraph(QPen(Qt::red, 3));
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
