#ifndef PROPERTIESTABLEFORM_H
#define PROPERTIESTABLEFORM_H

#include <QDialog>

namespace Ui {
class PropertiesTableForm;
}

class PropertiesTableForm : public QDialog
{
    Q_OBJECT

public:
    explicit PropertiesTableForm(QWidget *parent = nullptr);
    ~PropertiesTableForm();

private:
    Ui::PropertiesTableForm *ui;
};

#endif // PROPERTIESTABLEFORM_H
