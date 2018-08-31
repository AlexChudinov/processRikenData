#include "CreditsDialog.h"
#include "ui_CreditsDialog.h"

CreditsDialog::CreditsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreditsDialog)
{
    ui->setupUi(this);
    setModal(true);
}

CreditsDialog::~CreditsDialog()
{
    delete ui;
}
