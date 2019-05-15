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

#include "Base/BaseObject.h"
#include "Plot/DataPlot.h"
#include "Plot/PlotPair.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mProgressBar(new QProgressBar)
{
    ui->setupUi(this);
    setCentralWidget(ui->mdiArea);
    mProgressBar->hide();
    statusBar()->addWidget(mProgressBar);
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
        "Riken Data (*.lst);;"
        "Riken ASCII Data (*.dat);;"
        "SPAMS file (*.atof)"
    );
    if(!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        QString ext = fileInfo.suffix();
        if(fileInfo.suffix() == "lst")
        {
            openRikenDataFile(fileName);
        }
        else if(fileInfo.suffix() == "dat")
        {
            openRikenASCIIData(fileName);
        }
        else if(fileInfo.suffix() == "atof")
        {
            openSpamsFile(fileName);
        }
    }
}

void MainWindow::openRikenDataFile(const QString &fileName)
{
    createTicAndMsGraphs();
    Reader * reader = new RikenFileReader;
    reader->open(fileName);
    QThreadPool::globalInstance()->start(reader);
}

void MainWindow::openRikenASCIIData(const QString &fileName)
{
    createTicAndMsGraphs();
    Reader * reader = new DirectMsFromRikenTxt;
    reader->open(fileName);
    connect(reader, SIGNAL(progressNotify(int)), mProgressBar.data(), SLOT(setValue(int)));
    QThreadPool::globalInstance()->start(reader);
    mProgressBar->show();
    connect(reader, SIGNAL(finished()), mProgressBar.data(), SLOT(hide()));
}

void MainWindow::openRikenDataFiles(const QStringList &fileNames)
{
    QProgressBar * progress = new QProgressBar;
    progress->setTextVisible(true);
    progress->setMinimum(0);
    progress->setMaximum(fileNames.size());
    statusBar()->addWidget(progress);
    createTicAndMsGraphs();
    Reader * reader = new RikenFileReader;
    MyInit::instance()->timeEvents()->blockingClear();
    reader->disconnect(SIGNAL(started()), MyInit::instance()->timeEvents(), SLOT(blockingClear()));
    for(int i = 0; i < fileNames.size(); ++i)
    {
        QFileInfo fileInfo(fileNames[i]);
        progress->setFormat(tr("Loading ") + fileInfo.fileName());
        progress->setValue(i);
        reader->open(fileNames[i]);
        reader->run();
        reader->close();
    }
    statusBar()->removeWidget(progress);
}

void MainWindow::createTicAndMsGraphs()
{
    ui->mdiArea->closeAllSubWindows();

    PlotPair * plots = new PlotPair;

    connect
    (
        plots,
        SIGNAL(dataSelected(QVector<double>, QVector<double>, QString, QString)),
        this,
        SLOT(createDataPlot(QVector<double>, QVector<double>, QString, QString))
    );

    ui->mdiArea->addSubWindow(plots)->show();

    on_actionTileSubWindows_triggered();
}

void MainWindow::openSpamsFile(const QString &fileName)
{
    MyInit::instance()->massSpecColl()->setMsType(MassSpecImpl::MassSpecVecType);
    createTicAndMsGraphs();
    Reader * reader = new SPAMSHexinDataX32;
    mProgressBar->show();
    reader->open(fileName);
    QThreadPool::globalInstance()->start(reader);
    connect(reader, SIGNAL(progress(int)), mProgressBar.data(), SLOT(setValue(int)));
    connect(reader, SIGNAL(finished()), mProgressBar.data(), SLOT(hide()));
}

void MainWindow::on_actionTileSubWindows_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionReaccumulate_mass_spectra_triggered()
{
    int val = QInputDialog::getInt
    (
        this,
        "Ion counter",
        "Number of starts:",
        static_cast<int>(MyInit::instance()->timeEvents()->startsPerHist())
    );

    createTicAndMsGraphs();

    MyInit::instance()->timeEvents()->recalculateTimeSlices(static_cast<size_t>(val));

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

void MainWindow::on_actionOpen_txt_from_folder_triggered()
{
    QString dir = QFileDialog::getExistingDirectory
    (
        this,
        tr("Open directory"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if(!dir.isEmpty())
    {
        bool ok;
        createTicAndMsGraphs();
        int scope = QInputDialog::getInt
        (
            this,
            "Intensity scope",
            "Input intensity scope",
            10,
            10,
            std::numeric_limits<int>::max(),
            1,
            &ok
        );
        scope = ok ? scope : 1000;
        Reader * reader = new TxtFileReader(scope);
        reader->open(dir);
        QThreadPool::globalInstance()->start(reader);
    }
}

void MainWindow::on_actionTime_params_triggered()
{
    TimeParams* params = MyInit::instance()->timeParams();
    QMapPropsDialog dlg;
    dlg.setProps(params->get());
    dlg.exec();
    if(dlg.result() == QDialog::Accepted)
    {
        QVariantMap props = dlg.props();
        params->set(props);
    }
}

void MainWindow::on_actionopenManyBinFiles_triggered()
{
    QStringList files = QFileDialog::getOpenFileNames
    (
        this,
        "Open file",
        QString(),
        "Riken Data (*.lst)"
    );
    if(!files.isEmpty())
    {
        QFileInfo file(files[0]);
        QString ext = file.suffix();
        if(ext == "lst")
        {
            openRikenDataFiles(files);
        }
    }
}

void MainWindow::createDataPlot(QVector<double> x, QVector<double> y, QString capture, QString xLabel)
{
    QMdiSubWindow * w = ui->mdiArea->addSubWindow(new DataPlot(x, y, capture, xLabel));
    w->setWindowTitle(capture);
    w->show();
    on_actionTileSubWindows_triggered();
}

void MainWindow::on_actionReal_precision_triggered()
{
    bool ok = true;;
    int nDigits = QInputDialog::getInt
    (
        this,
        tr("Real number precision"),
        tr("Precision"),
        6,
        0,
        16,
        1,
        &ok
    );
    if(ok) MyInit::instance()->setPrecision(nDigits);
}
