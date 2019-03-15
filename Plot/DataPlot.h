#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QMainWindow>
#include <QPointer>
#include <PropertiesListForm.h>

#include "Data/plotdata.h"
#include "Math/Smoother.h"
#include "Math/interpolator.h"

class BasePlot;

/**
 * @brief The PropertiesOfPlot class to manage plots at the data plot
 */
class PropertiesOfPlot : public PropertiesListForm
{
    Q_OBJECT
public:
    PropertiesOfPlot(QWidget * parent= Q_NULLPTR);
    ~PropertiesOfPlot();
};

class DataPlot : public QMainWindow
{
    Q_OBJECT
public:
    explicit DataPlot
    (
        const QVector<double>& x,
        const QVector<double>& y,
        const QString& capture,
        const QString& xLabel,
        const QString& yLabel = QString(),
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

    void onShowMs();

    void on_showPlotsTriggered();
private:
    const static QList<Qt::GlobalColor> s_colors;

    QPointer<BasePlot> mPlot;

    QScopedPointer<PlotData> mData;

    Interpolator::Pointer mInterp;
    Smoother::Pointer mSmoother;
};

#endif // DATAPLOT_H
