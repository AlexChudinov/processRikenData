#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMdiSubWindow>
#include <QFileDialog>
#include <QMessageBox>
#include "Data/Reader.h"

#include "DialogAbout.h"
#include "QMapPropsDialog.h"
#include "CreditsDialog.h"
#include "PropertiesListForm.h"
#include "MassSpecAccDialogs.h"
#include "RikenData/rawrikendata.h"
#include "Plot/MSPlot.h"
#include "Plot/TICPlot.h"
#include "Base/BaseObject.h"
#include "Data/MassSpec.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->mdiArea);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::msg(const QString &msg)
{
    QMessageBox::warning(Q_NULLPTR, "Ion counter", msg);
}


void MainWindow::on_actionOpenDataFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName
    (
        this,
        "Open file",
        QString(),
        "Riken Data (*.lst)"
    );
    if(!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        QString ext = fileInfo.suffix();
        if(fileInfo.suffix() == "lst")
        {
            openRikenDataFile(fileName);
        }
    }
}

void MainWindow::openRikenDataFile(const QString &fileName)
{
    createTicAndMsGraphs();
    Reader * reader = new RikenFileReader;
    reader->open(fileName);
    QThreadPool::globalInstance()->start(reader);
    on_actionTileSubWindows_triggered();
}

void MainWindow::createTicAndMsGraphs()
{
    ui->mdiArea->closeAllSubWindows();

    MSPlot * msPlot = new MSPlot;
    TICPlot * ticPlot = new TICPlot;

    ui->mdiArea->addSubWindow(msPlot)->show();
    ui->mdiArea->addSubWindow(ticPlot)->show();

    connect
    (
        MyInit::instance()->massSpec(), SIGNAL(massSpecsNumNotify(size_t)),
        msPlot, SLOT(updateLast(size_t))
    );
    connect
    (
        MyInit::instance()->massSpec(), SIGNAL(massSpecsNumNotify(size_t)),
        ticPlot, SLOT(updateLast(size_t))
    );
    connect
    (
        ticPlot, SIGNAL(cursorPosNotify(size_t)),
        msPlot, SLOT(showMassSpec(size_t))
    );
    connect
    (
        ticPlot, SIGNAL(msLimitsNotify(size_t, size_t)),
        msPlot, SLOT(setLimits(size_t, size_t))
    );
    on_actionTileSubWindows_triggered();
}

void MainWindow::on_actionTileSubWindows_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionReaccumulate_mass_spectra_triggered()
{
    ui->mdiArea->closeAllSubWindows();

    int val = QInputDialog::getInt
    (
        this,
        "Ion counter",
        "Number of starts:",
        static_cast<int>(MyInit::instance()->timeEvents()->startsPerHist())
    );

    createTicAndMsGraphs();

    MyInit::instance()->timeEvents()->recalculateTimeSlices(static_cast<size_t>(val));

    on_actionTileSubWindows_triggered();
}

void MainWindow::on_actionCredits_triggered()
{
    CreditsDialog dlg(this);
    dlg.exec();
    dlg.show();
}



void MainWindow::on_actionAbout_triggered()
{
    DialogAbout dlg(this);
    dlg.exec();
    dlg.show();
}
