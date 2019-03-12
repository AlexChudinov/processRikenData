#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QMainWindow>
#include <QPointer>
#include "Math/Smoother.h"
#include "Math/interpolator.h"

class BasePlot;

class DataPlot : public QMainWindow
{
    Q_OBJECT
public:
    explicit DataPlot
    (
        const QVector<double>& x,
        const QVector<double>& y,
        const QString& capture,
        QWidget *parent = Q_NULLPTR
    );

signals:

public slots:
    /**
     * @brief calculateSmoothing estimates data smoothing and adds corresponding graph
     */
    void calculateSmoothing();

    void createSmoother();

    void chooseInterpolator();
private:
    QPointer<BasePlot> mPlot;

    Interpolator::Pointer mInterp;
    Smoother::Pointer mSmoother;
};

#endif // DATAPLOT_H
