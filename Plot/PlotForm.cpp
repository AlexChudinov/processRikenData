#include "Math/Smoother.h"
#include "PlotForm.h"
#include "ui_PlotForm.h"
#include "PropertiesListForm.h"

#include "QMapPropsDialog.h"
#include <QToolBar>
#include <algorithm>
#include <QFileDialog>
#include <QInputDialog>

PlotForm::PlotForm(const CompressedMS &ms, const QString& strDscrpt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotForm),
    m_pPlot(new QCustomPlot(this)),
    m_pMassSpec(new CompressedMS(ms)),
    m_props(new PropertiesListForm())
{
    m_props->setWindowTitle("Props: " + windowTitle());
    connect(m_pPlot, SIGNAL(mouseMove(QMouseEvent*)),
            this, SLOT(printMouseCoordinates(QMouseEvent*)));
    connect(m_pPlot, SIGNAL(mousePress(QMouseEvent*)),
            this, SLOT(mouseClick(QMouseEvent*)));
    ui->setupUi(this);
    QToolBar * toolBar = new QToolBar(this);
    setUpToolBar(toolBar);
    ui->verticalLayout->addWidget(toolBar);
    ui->verticalLayout->addWidget(m_pPlot);

    QString title = this->windowTitle();
    (title += ": ") += strDscrpt;
    this->setWindowTitle(title);
    m_nXMin = m_pMassSpec->interp()->minX();
    m_nXMax = m_pMassSpec->interp()->maxX();
    m_nYMin = m_pMassSpec->interp()->minY();
    m_nYMax = m_pMassSpec->interp()->maxY();
    addMassSpecGraph();
    connect(m_pPlot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            this, SLOT(adjustRangeToLimits(QCPRange)));
    connect(m_pPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
            this, SLOT(adjustRangeToLimits(QCPRange)));
    m_pPlot->xAxis->setTickLabelFont(QFont("Arial", 12));
    m_pPlot->yAxis->setTickLabelFont(QFont("Arial", 12));
    m_pPlot->xAxis->setLabelFont(QFont("Arial", 12));
    m_pPlot->yAxis->setLabelFont(QFont("Arial", 12));
    m_pPlot->xAxis->setLabel("time [bin]");
    m_pPlot->yAxis->setLabel("ion count");

    fillPropertiesList();
}

PlotForm::~PlotForm()
{
    delete ui;
}

void PlotForm::addMassSpecGraph()
{
    QCPGraph * graph = m_pPlot->addGraph();
    graph->setPen(QPen(Qt::blue, 2));
    addCompressedDataToGraph(graph, m_pMassSpec.data());
    m_pPlot->rescaleAxes();
    m_pPlot->replot();
}

void PlotForm::addSmoothedGraph()
{
    if(m_pPlot->graphCount() == 2)
    {
        m_pPlot->removeGraph(1);
    }
    QCPGraph * graph = m_pPlot->addGraph();
    graph->setPen(QPen(Qt::red, 3));
    addCompressedDataToGraph(graph, m_pSmoothedData.data());
    m_pPlot->replot();
}

void PlotForm::adjustRangeToLimits(QCPRange)
{
    QCPRange xrange = m_pPlot->xAxis->range();
    xrange.lower = qMax(xrange.lower, static_cast<double>(m_nXMin));
    xrange.upper = qMin(xrange.upper, static_cast<double>(m_nXMax));
    m_pPlot->xAxis->setRange(xrange);

    QSharedPointer<QCPGraphDataContainer> data
            = m_pPlot->graph(0)->data();
    using Iterator = QCPGraphDataContainer::const_iterator;
    double ymax = static_cast<double>(m_nYMin);
    double ymin = static_cast<double>(m_nYMax);
    Iterator
            First = data->findBegin(xrange.lower),
            Last = data->findEnd(xrange.upper);
    for(Iterator i = First; i < Last; ++i)
    {
        ymax = qMax(ymax, i->value);
        ymin = qMin(ymin, i->value);
    }
    m_pPlot->yAxis->setRange(ymin, ymax * 1.1);
    m_pPlot->replot();
}

void PlotForm::msg(const QString &msg)
{
    QMessageBox::warning
    (
        this,
        "Plot Message",
        msg
    );
}

void PlotForm::setUpToolBar(QToolBar * toolBar)
{
    toolBar->addActions
    (
        {
            ui->actionHorizontalZoom,
            ui->actionZoomOut,
            ui->actionDragXAxisLim
        }
    );
    toolBar->addSeparator();
    toolBar->addActions
    (
        {
            ui->actionSplineSmoothing
        }
    );
    toolBar->addSeparator();
    toolBar->addActions
    (
        {
            ui->actionSavePicture,
            ui->actionImport,
            ui->actionMassSpecProps
        }
    );
}

void PlotForm::addCompressedDataToGraph(QCPGraph *g, const CompressedMS *ms) const
{
    QVector<double> x(ms->interp()->table().size());
    QVector<double> y(ms->interp()->table().size());

    CompressedMS::Map::const_iterator it = ms->interp()->table().begin();
    for(size_t i = 0; i < ms->interp()->table().size(); ++i)
    {
        x[i] = it->first * ms->interp()->xFactor();
        y[i] = it->second * ms->interp()->yFactor();
        ++it;
    }

    g->addData(x, y);
}

void PlotForm::importTextDataToFile(QTextStream &out, const QCPGraphDataContainer * tab) const
{
    QCPRange r = m_pPlot->xAxis->range();
    using Iterator = QCPGraphDataContainer::const_iterator;
    Iterator
            First = tab->findBegin(r.lower),
            Last = tab->findEnd(r.upper);
    out << "#x:" << "\t\t" << "#y:" << "\n";
    for(Iterator i = First; i < Last; ++i)
    {
        size_t key = static_cast<size_t>(i->key);
        size_t val = static_cast<size_t>(i->value);
        out << key << "\t" << val << "\n";
    }
}

void PlotForm::fillPropertiesList()
{
    m_props->clearList();
    m_props->addListEntry
    (
        QString("TIC: %1").arg(m_pMassSpec->totalIonCount())
    );
    m_props->addListEntry
    (
        QString("Min time count: %1")
                .arg(m_pMassSpec->interp()->minX())
    );
    m_props->addListEntry
    (
        QString("Max time count: %1")
                .arg(m_pMassSpec->interp()->maxX())
    );
    if(m_pSmoothedData)
    {
        m_props->addListEntry
        (
            QString("Smooth. param.: %1")
                    .arg(m_smoothParam)
        );
    }
    if(m_peaks)
    {
        m_props->addListEntry
        (
            QString("Number of detected peaks: %1")
                    .arg(m_peaks->size())
        );
    }
}

void PlotForm::on_actionHorizontalZoom_triggered()
{
    m_pPlot->setSelectionRectMode(QCP::srmZoom);
    m_pPlot->setCursor(QCursor(QPixmap("://Icons//zoomIn")));
}

void PlotForm::printMouseCoordinates(QMouseEvent *event)
{
    double x = m_pPlot->xAxis->pixelToCoord(event->pos().x());
    double y = m_pPlot->yAxis->pixelToCoord(event->pos().y());
    QString strMousePos = QString("x: %1, y: %2")
            .arg(x, 0, 'E', 8)
            .arg(y);
    Q_EMIT mouseCoordNotify(strMousePos);
}

void PlotForm::mouseClick(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        m_pPlot->setSelectionRectMode(QCP::srmNone);
        m_pPlot->setCursor(QCursor(Qt::ArrowCursor));
    }
    else
    {
        if(m_pPlot->cursor().shape() == Qt::OpenHandCursor)
        {
            QPoint pt = event->pos();
            double x = m_pPlot->xAxis->pixelToCoord(pt.x());
            QCPRange range = m_pPlot->xAxis->range();
            double x0 = .5 * (range.upper + range.lower);
            if(x < x0)
                (range.lower *= 2) -= x;
            else
                (range.upper *= 2) -= x;
            range.lower = qMax(range.lower, static_cast<double>(m_nXMin));
            range.upper = qMin(range.upper, static_cast<double>(m_nXMax));
            m_pPlot->xAxis->setRange(range);
        }
    }
}

void PlotForm::on_actionZoomOut_triggered()
{
    //Do not adjust limits when it is zoom out
    m_pPlot->xAxis->blockSignals(true);
    m_pPlot->xAxis->blockSignals(true);
    //
    m_pPlot->setSelectionRectMode(QCP::srmNone);
    m_pPlot->setCursor(QCursor(Qt::ArrowCursor));
    m_pPlot->rescaleAxes();
    m_pPlot->replot();
    m_pPlot->xAxis->blockSignals(false);
    m_pPlot->xAxis->blockSignals(false);
}

void PlotForm::on_actionImport_triggered()
{
    QStringList items{"Raw data", "Smoothed data", "Peaks"};
    bool ok;
    QString strData = QInputDialog::getItem
    (
        this,
        "Plot Dialog",
        "Choose the data",
         items, 0, false, &ok
    );
    if(ok && !strData.isEmpty())
    {
        QString importFileName = QFileDialog::getSaveFileName(this,
                                 "Choose the name of file to import");

        if(!importFileName.isEmpty())
        {
            QFile file(importFileName);
            if(file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);
                if(strData == "Raw data")
                {
                    QSharedPointer<QCPGraphDataContainer> graphData
                            = m_pPlot->graph(0)->data();
                    importTextDataToFile(out, graphData.data());
                }
                if(strData == "Smoothed data" && m_pPlot->graphCount() == 2)
                {
                    QSharedPointer<QCPGraphDataContainer>  graphData
                            = m_pPlot->graph(1)->data();
                    importTextDataToFile(out, graphData.data());
                }
                if(strData == "Peaks" && m_peaks)
                {
                    QCPRange r = m_pPlot->xAxis->range();
                    Peak::PeakCollection::const_iterator First
                            = m_peaks->lower_bound(r.lower);
                    Peak::PeakCollection::const_iterator Last
                            = m_peaks->lower_bound(r.upper);
                    out << "#Center: \t" << "#Height: \t"
                        << "#Left: \t" << "#Right: \n";
                    out << qSetRealNumberPrecision(10);
                    for(; First != Last; ++First)
                    {
                        out << First->center() << "\t" << First->height() << "\t"
                            << First->left() << "\t" << First->right() << "\n";
                    }
                }
                file.close();
            }
            else
            {
                msg("Failed to open file!");
            }
        }
    }
}

void PlotForm::on_actionSplineSmoothing_triggered()
{
    QStringList items
    {
        "Parametric spline",
        "Parameterless Poisson noise spline",
        "Peak counter spline"
    };
    bool ok;
    QString strData = QInputDialog::getItem
    (
        this,
        "Plot Dialog",
        "Type of smoothing",
         items, 0, false, &ok
    );

    if(ok && !strData.isEmpty())
    {
        Smoother::Pointer calc;
        if(strData == items[0])
            calc.reset
            (
                Smoother::create
                (
                    Smoother::LogSplinePoissonWeightType
                ).release()
            );
        else if(strData == items[1])
            calc.reset
            (
                Smoother::create
                (
                    Smoother::LogSplinePoissonWeightPoissonNoiseType
                ).release()
            );
        else if(strData == items[2])
            calc.reset
            (
                Smoother::create
                (
                    Smoother::LogSplinePoissonWeightOnePeakType
                ).release()
            );

        QCPRange range = m_pPlot->xAxis->range();
        m_pSmoothedData.reset
        (
            new CompressedMS
            (
                m_pMassSpec->cutRange(range.lower, range.upper)
            )
        );

        QMapPropsDialog dialog;
        dialog.setProps(calc->params());
        dialog.exec();

        if(dialog.result() == QDialog::Accepted)
        {
            calc->setParams(dialog.props());
            m_peaks.reset
            (
                new Peak::PeakCollection
                (
                    m_pSmoothedData->smooth(calc.get())
                )
            );
            addSmoothedGraph();
            fillPropertiesList();
        }
    }
}

void PlotForm::on_actionSavePicture_triggered()
{
    QString fileFilt;
    QString fileName = QFileDialog::getSaveFileName
    (
        this,
        "Save as an image",
        QString(),
        tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;PDF (*.pdf)"),
        &fileFilt
    );
    if(!fileName.isEmpty())
    {
        if(fileFilt == "PNG (*.png)") m_pPlot->savePng(fileName);
        if(fileFilt == "JPG (*.jpg)") m_pPlot->saveJpg(fileName);
        if(fileFilt == "BMP (*.bmp)") m_pPlot->saveBmp(fileName);
        if(fileFilt == "PDF (*.pdf)") m_pPlot->savePng(fileName);
    }
}

void PlotForm::on_actionDragXAxisLim_triggered()
{
    m_pPlot->setCursor(QCursor(Qt::OpenHandCursor));
}

void PlotForm::on_actionMassSpecProps_triggered()
{
    m_props->show();
}
