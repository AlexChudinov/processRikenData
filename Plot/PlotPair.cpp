#include "PlotPair.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"
#include "Data/TimeEvents.h"
#include "Data/XValsTransform.h"
#include <QInputDialog>

PlotPair::PlotPair(QWidget *parent) :
    QMainWindow(parent),
    mMsPlot(new BasePlot(this)),
    mTicPlot(new BasePlot(this)),
    mXValsTransform(new TimeScale())
{
    createGraphs();

    MassSpec * ms = MyInit::instance()->massSpec();
    TimeParams * timeParams = MyInit::instance()->timeParams();
    connect(ms, SIGNAL(cleared()), SLOT(clearData()));
    massSpecNumsChanged(ms->blockingSize());
    connect
    (
        ms, SIGNAL(massSpecsNumNotify(size_t)),
        SLOT(massSpecNumsChanged(size_t))
    );
    connect(timeParams, SIGNAL(setParamsNotify()), SLOT(onShowMs()));

    setStatusBar(new QStatusBar);
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

PlotPair::~PlotPair()
{
    delete mXValsTransform;
}

void PlotPair::setTicCursorPos(double x)
{
    MassSpec * ms = MyInit::instance()->massSpec();

    x = std::round(x);
    if(x < 0.0) x = 0.0;
    if(x >= ms->blockingSize()) x = ms->blockingSize() - 1;

    if(x < static_cast<double>(mTicPlot->graph(0)->data()->size()))
    {
        double ymax = std::lower_bound
        (
            mTicPlot->graph(0)->data()->constBegin(),
            mTicPlot->graph(0)->data()->constEnd(),
            QCPGraphData::fromSortKey(x),
            qcpLessThanSortKey<QCPGraphData>
        )->value;
        mTicPlot->graph(1)->data()->set({{x, 0.}, {x, ymax}});
    }
    else
    {
        size_t curMsNum = static_cast<size_t>(mTicPlot->graph(0)->data()->size());
        size_t idx = static_cast<size_t>(x);
        double ticIdx = 0.0, tic = 0.0;
        for(;curMsNum <= idx; ++curMsNum)
        {
            ticIdx = static_cast<double>(curMsNum);
            tic = ms->blockingGetMassSpecTotalCurrent(curMsNum);
            mTicPlot->graph(0)->data()->add({ticIdx, tic});
        }
        mTicPlot->graph(1)->data()->set({{ticIdx, 0.}, {ticIdx, tic}});
        mTicPlot->rescaleAxes();
    }
    mTicPlot->replot();

    onShowMs();
}

void PlotPair::setTicCursorPos(QMouseEvent *evt)
{
    if
    (
        evt->button() == Qt::LeftButton
        && mTicPlot->cursor().shape() == Qt::ArrowCursor
    )
    {
        setTicCursorPos(mTicPlot->xAxis->pixelToCoord(evt->pos().x()));
    }
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

void PlotPair::onShowMs()
{
    MassSpec * ms = MyInit::instance()->massSpec();
    if(ms->blockingSize() == 0) return;
    int idx = qRound(mTicPlot->graph(1)->data()->begin()->key);
    MapUintUint MS = ms->blockingGetMassSpec(static_cast<size_t>(idx));
    QSharedPointer<QCPGraphDataContainer> msData(new QCPGraphDataContainer);
    for(MapUintUint::const_reference d : MS)
    {
        msData->add
        (
            {
                mXValsTransform->transform(static_cast<double>(d.first)),
                static_cast<double>(d.second)
            }
        );
    }
    mMsPlot->graph(0)->setData(msData);
    mMsPlot->xAxis->setLabel(mXValsTransform->xUnits());
    if(idx == mTicPlot->graph(0)->data()->size() - 1 && mTicPlot->graph(0)->data()->size() == 1)
    { //Rescale only first time
        mMsPlot->rescaleAxes();
    }
    mMsPlot->replot();
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

    connect(mTicPlot.data(), SIGNAL(mouseCoordinateNotify(QString)),
            statusBar(), SLOT(showMessage(QString)));

    connect(mMsPlot.data(), SIGNAL(mouseCoordinateNotify(QString)),
            statusBar(), SLOT(showMessage(QString)));

    connect(mTicPlot.data(), SIGNAL(mousePress(QMouseEvent*)),
            SLOT(setTicCursorPos(QMouseEvent*)));
}
