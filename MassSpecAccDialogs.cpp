#include "MassSpecAccDialogs.h"
#include "ui_SimpleMassSpecAccDialog.h"
#include "ui_AccumScaleCorrectionDialog.h"

SimpleMassSpecAccDialog::SimpleMassSpecAccDialog(int nMaxValue, QWidget *parent)
    :
    QDialog(parent),
    ui(new Ui::SimpleMassSpecAccDialog)
{
    ui->setupUi(this);
    setModal(true);
    setFixedSize(this->width(), this->height());
    ui->minIdxSpinBox->setRange(0, nMaxValue);
    ui->maxIdxSpinBox->setRange(0, nMaxValue);
    ui->minIdxSpinBox->setValue(0);
    ui->maxIdxSpinBox->setValue(nMaxValue);
}

SimpleMassSpecAccDialog::~SimpleMassSpecAccDialog()
{
    delete ui;
}

QPair<int, int> SimpleMassSpecAccDialog::getAccumulationLimits() const
{
    return QPair<int, int>(ui->minIdxSpinBox->value(), ui->maxIdxSpinBox->value());
}


AccumScaleCorrectionDialog::AccumScaleCorrectionDialog(int nSweepsNum, QWidget *parent)
    :
      QDialog(parent),
      ui(new Ui::AccumScaleCorrectionDialog)
{
    ui->setupUi(this);
    setModal(true);
    setFixedSize(this->width(), this->height());
    ui->minSweepIdxSpinBox->setRange(0, nSweepsNum);
    ui->maxSweepIdxSpinBox->setRange(0, nSweepsNum);
    ui->stepSizeSpinBox->setRange(0, nSweepsNum);
    ui->minSweepIdxSpinBox->setValue(0);
    ui->maxSweepIdxSpinBox->setValue(nSweepsNum);
    ui->stepSizeSpinBox->setValue(nSweepsNum/10);
}

AccumScaleCorrectionDialog::~AccumScaleCorrectionDialog()
{
    delete ui;
}
