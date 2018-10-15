#ifndef MSPLOT_H
#define MSPLOT_H

#include <QPointer>
#include <QMainWindow>

class BasePlot;

class MSPlot : public QMainWindow
{
    Q_OBJECT

public:
    MSPlot(QWidget * parent = Q_NULLPTR);

    Q_SLOT void plotMS(size_t idx);
private:
    QPointer<BasePlot> mPlot;
};

#endif // MSPLOT_H
