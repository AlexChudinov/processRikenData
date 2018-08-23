#include "MassSpecAccDialogs.h"
#include "ui_SimpleMassSpecAccDialog.h"

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
