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
    MassSpectrumsCollection * ms = MyInit::instance()->massSpecColl();
    setWindowTitle(ms->fileName());
    connect(ms, SIGNAL(fileNameNotify(QString)), SLOT(setWindowTitle(QString)));
    TimeParams * timeParams = MyInit::instance()->timeParams();
    connect(ms, SIGNAL(cleared()), SLOT(clearData()));
    massSpecNumsChanged(ms->blockingSize());
    connect
    (
        ms, SIGNAL(massSpecNumNotify(size_t)),
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

    mMsPlot->toolBar()->addAction
    (
        QIcon("://Icons//selectMS"),
        "Select data",
        this,
        SLOT(onSelectData())
    );

    mTicPlot->xAxis->setLabel(tr("Mass spectrum number"));

    addToolBar(Qt::TopToolBarArea, mMsPlot->toolBar());

    connectActions();
}

PlotPair::~PlotPair()
{
    delete mXValsTransform;
}

void PlotPair::setTicCursorPos(double x)
{
    MassSpectrumsCollection * ms = MyInit::instance()->massSpecColl();

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
        size_t idx = ms->blockingSize();
        VecInt tic = ms->readTotalIonCurrent(curMsNum, idx);
        QVector<double> keys(idx - curMsNum);
        std::iota(keys.begin(), keys.end(), curMsNum);
        QVector<double> vals(idx - curMsNum);
        std::transform
        (
            tic.begin(),
            tic.end(),
            vals.begin(),
            [](int val)->double
            {
                return static_cast<double>(val);
            }
        );
        mTicPlot->graph(0)->addData(keys, vals);
        mTicPlot->graph(1)->data()->set({{keys.back(), 0.}, {keys.back(), vals.back()}});
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
    if(massSpecNum != 0 && massSpecNum > mTicPlot->graph(0)->data()->size())
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
    MassSpectrumsCollection * ms = MyInit::instance()->massSpecColl();
    if(ms->blockingSize() == 0) return;
    int idx = qRound(mTicPlot->graph(1)->data()->begin()->key);
    MassSpecImpl::MapShrdPtr MS = ms->blockingMassSpec(static_cast<size_t>(idx));
    QSharedPointer<QCPGraphDataContainer> msData(new QCPGraphDataContainer);
    for(MassSpecImpl::Map::const_reference d : * MS)
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
    if(idx == 0 && mTicPlot->graph(0)->data()->size() == 1)
    { //Rescale only first time
        mMsPlot->rescaleAxes();
    }
    mMsPlot->replot();
}

void PlotPair::onSelectData()
{
    mTicPlot->setCursor(Qt::PointingHandCursor);
    mMsPlot->setCursor(Qt::PointingHandCursor);
    mTicPlot->setSelectionRectMode(QCP::srmCustom);
    mMsPlot->setSelectionRectMode(QCP::srmCustom);
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

    connect(mMsPlot, SIGNAL(mouseRelease(QMouseEvent *)),
            this, SLOT(selectMsData()));

    connect(mTicPlot, SIGNAL(mouseRelease(QMouseEvent *)),
            this, SLOT(selectTicData()));
}

void PlotPair::selectMsData()
{
    if(mMsPlot->selectionRectMode() == QCP::srmCustom)
    {
        const QCPSelectionRect * rect = mMsPlot->selectionRect();
        QCPRange xrange = rect->range(mMsPlot->xAxis);
        int minX = static_cast<int>(mXValsTransform->invTransform(::round(xrange.lower)));
        int maxX = static_cast<int>(mXValsTransform->invTransform(::round(xrange.upper)));

        MassSpectrumsCollection * msColl = MyInit::instance()->massSpecColl();
        const int n = msColl->blockingSize<int>();
        QVector<double> x(n), y(n);
        double x0 = 0;
        for(int i = 0; i < n; ++i)
        {
            MassSpecImpl::MapShrdPtr msDataPtr = msColl->blockingMassSpec(i);
            x[i] = ++x0;
            y[i] = 0.0;
            MassSpecImpl::Map::const_iterator
                    _First = msDataPtr->lower_bound(minX),
                    _Last = msDataPtr->upper_bound(maxX);
            for(; _First != _Last; ++_First)
            {
                y[i] += _First->second;
            }
        }

        Q_EMIT dataSelected
        (
            x,
            y,
            tr("TIC for %3: %1 - %2").arg(minX).arg(maxX).arg(mXValsTransform->xUnits()),
            mTicPlot->xAxis->label()
        );
    }
}

void PlotPair::selectTicData()
{
    if(mTicPlot->selectionRectMode() == QCP::srmCustom)
    {
        MassSpectrumsCollection * ms = MyInit::instance()->massSpecColl();
        const size_t n = ms->blockingSize();
        const QCPSelectionRect * rect = mTicPlot->selectionRect();
        QCPRange xrange = rect->range(mTicPlot->xAxis);
        size_t minX = xrange.lower >= 0 ? static_cast<size_t>(xrange.lower) : 0;
        size_t maxX = xrange.upper < n ? static_cast<size_t>(xrange.upper + 1) : n;

        MapIntInt msDataMap = DirectSum().accum(ms, minX, maxX);

        QVector<double> x(msDataMap.size()), y(msDataMap.size());

        int i = 0;
        for(MapIntInt::const_reference d : msDataMap)
        {
            x[i] = d.first;
            y[i] = d.second;
            ++i;
        }

        Q_EMIT dataSelected
        (
            x,
            y,
            tr("MS indexes: %1 - %2").arg(minX).arg(maxX),
            mMsPlot->xAxis->label()
        );
    }
}
