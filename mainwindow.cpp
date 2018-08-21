#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "rikendataheaderform.h"
#include "RikenData/rawrikendata.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pData(new RawRikenData(this)),
    m_pHeaderForm(new RikenDataHeaderForm())
{
    ui->setupUi(this);
    ui->actionShowDataHeader->setDisabled(true);
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
    readRikenDataFile();
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

    m_pHeaderForm->clearList();

    const QStringList& header = m_pData->getStringDataHeader();

    for(const auto& s : header)
    {
        m_pHeaderForm->addListEntry(s);
    }
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
