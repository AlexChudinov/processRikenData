#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMdiSubWindow>
#include <QFileDialog>
#include <QMessageBox>

#include "PropertiesListForm.h"
#include "MassSpecAccDialogs.h"
#include "RikenData/rawrikendata.h"
#include "Plot/PlotForm.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pData(new RawRikenData(this)),
    m_pHeaderForm(new PropertiesListForm()),
    m_pPropForm(new PropertiesListForm())
{
    ui->setupUi(this);
    setCentralWidget(ui->mdiArea);
    disableRikenDataFileActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionTileSubWindows_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionOpenRikenDataFile_triggered()
{
    m_strRikenFilePath =
            QFileDialog::getOpenFileName(this, tr("Open File"),
                                         QString(),
                                         tr("RIKEN Data (*.lst)"));

    if(!m_strRikenFilePath.isEmpty())
    {
        QFileInfo fileInfo(m_strRikenFilePath);
        m_strRikenFileName = fileInfo.fileName();
        readRikenDataFile();
    }
}

void MainWindow::readRikenDataFile()
{
    QFile file(m_strRikenFilePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        msg("Failed to open file!");
    }
    else
    {
        m_pData->readFile(file);
    }
    file.close();

    m_pHeaderForm->clearList();
    m_pPropForm->clearList();

    const QStringList& header = m_pData->getStringDataHeader();

    for(const auto& s : header)
    {
        m_pHeaderForm->addListEntry(s);
    }

    m_pPropForm->addListEntry(QString("Number of sweeps: %1")
                              .arg(m_pData->sweepsNumber()));
    m_pPropForm->addListEntry(QString("Max time bin: %1")
                              .arg(m_pData->maxTime()));
    m_pPropForm->addListEntry(QString("Min time bin: %1")
                              .arg(m_pData->minTime()));

    enableRikenDataFileActions();
}

void MainWindow::msg(const QString &msg)
{
    QMessageBox::warning(this,"Process Riken Data", msg);
}

void MainWindow::disableRikenDataFileActions()
{
    ui->actionShowDataHeader->setDisabled(true);
    ui->actionShowDataProperties->setDisabled(true);
    ui->actionSimpleMassSpecAcc->setDisabled(true);
    ui->actionTimeShiftAcc->setDisabled(true);
}

void MainWindow::enableRikenDataFileActions()
{
    ui->actionShowDataHeader->setEnabled(true);
    ui->actionShowDataProperties->setEnabled(true);
    ui->actionSimpleMassSpecAcc->setEnabled(true);
    ui->actionTimeShiftAcc->setEnabled(true);
}

void MainWindow::plotSubwindow(PlotForm *form)
{
    ui->mdiArea->addSubWindow(form)->show();
}

void MainWindow::on_actionShowDataHeader_triggered()
{
    m_pHeaderForm->show();
}

void MainWindow::on_actionShowDataProperties_triggered()
{
    m_pPropForm->show();
}

void MainWindow::on_actionSimpleMassSpecAcc_triggered()
{
    SimpleMassSpecAccDialog dialog(static_cast<int>(m_pData->sweepsNumber()));
    dialog.exec();
    if(dialog.result() == QDialog::Accepted)
    {
        QPair<int, int> pairAccLims = dialog.getAccumulationLimits();
        MassSpec massSpec = m_pData->accumulateMassSpec(pairAccLims.first,
                                                        pairAccLims.second);
        QString strDescription = m_strRikenFileName + tr(" sum [%1 .. %2]").
                arg(pairAccLims.first).arg(pairAccLims.second);
        plotSubwindow(new PlotForm(massSpec.compress(), strDescription));
    }
}

void MainWindow::on_actionTimeShiftAcc_triggered()
{
    AccumScaleCorrectionDialog dialog(static_cast<int>(m_pData->sweepsNumber()));
    dialog.exec();
    if(dialog.result() == QDialog::Accepted)
    {
        AccumScaleCorrectionDialog::DialogReturnParams params =
                dialog.getDialogParams();
        QString strDescr = m_strRikenFileName + tr(" sum with shift [%1 .. %2] step %3").
                arg(params.minSweepIdx).arg(params.maxSweepIdx).arg(params.step);
        CompressedMS ms = m_pData->accumulateMassSpec
        (
            params.minSweepIdx,
            params.maxSweepIdx,
            params.step
        );
        plotSubwindow(new PlotForm(ms, strDescr));
    }
}
