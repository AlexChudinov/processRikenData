#ifndef TICPLOT_H
#define TICPLOT_H
#include <QPointer>
#include <QMainWindow>

class BasePlot;

class TICPlot : public QMainWindow
{
    Q_OBJECT
public:
    TICPlot(QWidget * parent = Q_NULLPTR);

    Q_SLOT void updateLast();

    Q_SLOT void updateLimits(size_t first, size_t last);

    Q_SLOT void plot();
private:
    QPointer<BasePlot> mPlot;
    size_t mFirst;
    size_t mLast;
};

#endif // TICPLOT_H
