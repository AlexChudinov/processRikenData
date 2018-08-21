#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "rikendataheaderform.h"
#include "simplemassspecaccdialog.h"
#include "RikenData/rawrikendata.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pData(new RawRikenData(this)),
    m_pHeaderForm(new RikenDataHeaderForm()),
    m_pPropForm(new RikenDataHeaderForm())
{
    ui->setupUi(this);
    ui->actionShowDataHeader->setDisabled(true);
    ui->actionShowDataProperties->setDisabled(true);
    setCentralWidget(ui->mdiArea);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenRikenDataFile_triggered()
{
    m_StrRikenFileName =
            QFileDialog::getOpenFileName(this, tr("Open File"),
                                         QString(),
                                         tr("RIKEN Data (*.lst)"));

    if(!m_StrRikenFileName.isEmpty()) readRikenDataFile();
}

void MainWindow::readRikenDataFile()
{
    QFile file(m_StrRikenFileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        msg("Failed to open file!");
    }
    else
    {
        m_pData->readFile(file);
    }
    file.close();
    ui->actionShowDataHeader->setEnabled(true);
    ui->actionShowDataProperties->setEnabled(true);

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
}

void MainWindow::msg(const QString &msg)
{
    QMessageBox::warning(this,
                         "Process Riken Data Message",
                         msg);
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
        int n = 0;
    }
}
