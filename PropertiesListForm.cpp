#include "PropertiesListForm.h"
#include "ui_PropertiesListForm.h"

PropertiesListForm::PropertiesListForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertiesListForm)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);
    connect(ui->listWidget, SIGNAL(clicked(QModelIndex)),
            this, SLOT(emitItemStringFromIdx(QModelIndex)));
}

PropertiesListForm::~PropertiesListForm()
{
    delete ui;
}

void PropertiesListForm::addListEntry(const QString &entry)
{
    ui->listWidget->addItem(entry);
}

void PropertiesListForm::clearList()
{
    ui->listWidget->clear();
}

void PropertiesListForm::emitItemStringFromIdx(QModelIndex idx)
{
    Q_EMIT itemChosen(idx.data().toString());
}

