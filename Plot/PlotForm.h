#ifndef PLOTFORM_H
#define PLOTFORM_H

#include <QWidget>

namespace Ui {
class PlotForm;
}

class PlotForm : public QWidget
{
    Q_OBJECT

public:
    explicit PlotForm(QWidget *parent = 0);
    ~PlotForm();

private:
    Ui::PlotForm *ui;
};

#endif // PLOTFORM_H
