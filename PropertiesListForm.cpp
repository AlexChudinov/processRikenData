#include "rikendataheaderform.h"
#include "ui_rikendataheaderform.h"

RikenDataHeaderForm::RikenDataHeaderForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RikenDataHeaderForm)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);
}

RikenDataHeaderForm::~RikenDataHeaderForm()
{
    delete ui;
}

void RikenDataHeaderForm::addListEntry(const QString &entry)
{
    ui->listWidget->addItem(entry);
}

void RikenDataHeaderForm::clearList()
{
    ui->listWidget->clear();
}

