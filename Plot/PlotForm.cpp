#include "PlotForm.h"
#include "ui_PlotForm.h"

PlotForm::PlotForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotForm)
{
    ui->setupUi(this);
}

PlotForm::~PlotForm()
{
    delete ui;
}
