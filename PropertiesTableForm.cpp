#include "PropertiesTableForm.h"
#include "ui_PropertiesTableForm.h"

PropertiesTableForm::PropertiesTableForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesTableForm)
{
    ui->setupUi(this);
}

PropertiesTableForm::~PropertiesTableForm()
{
    delete ui;
}
