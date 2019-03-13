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
    explicit SimpleMassSpecAccDialog(int nMaxTime, QWidget *parent = Q_NULLPTR);
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

    explicit AccumScaleCorrectionDialog(int nSweepsNum, QWidget * parent = Q_NULLPTR);
    ~AccumScaleCorrectionDialog();

    struct DialogReturnParams
    {
        size_t minSweepIdx;
        size_t maxSweepIdx;
        size_t step;
        size_t peakWidth;
    };

    DialogReturnParams getDialogParams() const;

private:
    Ui::AccumScaleCorrectionDialog *ui;
};

#endif // SIMPLEMASSSPECACCDIALOG_H
