#ifndef SIMPLEMASSSPECACCDIALOG_H
#define SIMPLEMASSSPECACCDIALOG_H

#include <QDialog>

namespace Ui {
class SimpleMassSpecAccDialog;
}

class SimpleMassSpecAccDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimpleMassSpecAccDialog(int nMaxTime, QWidget *parent = 0);
    ~SimpleMassSpecAccDialog();

    QPair<int, int> getAccumulationLimits() const;

private:
    Ui::SimpleMassSpecAccDialog *ui;
};

namespace Ui {
class AccumScaleCorrectionDialog;
}

class AccumScaleCorrectionDialog : public QDialog
{
    Q_OBJECT

public:

    explicit AccumScaleCorrectionDialog(int nSweepsNum, QWidget * parent = 0);
    ~AccumScaleCorrectionDialog();

private:
    Ui::AccumScaleCorrectionDialog *ui;
};

#endif // SIMPLEMASSSPECACCDIALOG_H
