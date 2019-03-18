#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QMainWindow>
#include <QPointer>
#include <QColor>
#include <QDialog>

#include "Math/Smoother.h"
#include "Math/interpolator.h"
#include "../QCustomPlot/qcustomplot.h"

class BasePlot;

struct PropertiesOfPlot
{
    using GraphData = QSharedPointer<QCPGraphDataContainer>;
    QColor mColor;
    int mLineWidth;
    QString mDescription;
    GraphData mData;
};

/**
 * @brief The PropertiesOfPlot class to manage plots at the data plot
 */
class PropertiesOfPlotForm : public QDialog
{
    Q_OBJECT
public:
    PropertiesOfPlotForm(QWidget * parent= Q_NULLPTR);
    virtual ~PropertiesOfPlotForm();

    void addProps(const QList<PropertiesOfPlot>& props);

    void readProps(QList<PropertiesOfPlot>& props);

private slots:
    void chooseColor();
    void deletePlot();

private:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QDialogButtonBox *buttonBox;

    struct PropsWidgets
    {
        QLineEdit * mName;
        QSpinBox * mLineWidth;
        QPushButton * mColor;
        QPushButton * mDelete;
    };

    QMap<int, PropsWidgets> mWidgets;

    void addPropsEntry(int idx, const PropertiesOfPlot& property);
};

class DataPlot : public QMainWindow
{
    Q_OBJECT
public:

    using GraphData = PropertiesOfPlot::GraphData;
    using DoubleVector = QVector<double>;
    using GraphDataPtr = QSharedPointer<QCPGraphDataContainer>;

    explicit DataPlot
    (
        const DoubleVector& x,
        const DoubleVector& y,
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

    void on_showProps();
private:
    const static QList<Qt::GlobalColor> s_colors;

    QPointer<BasePlot> mPlot;

    Interpolator::Pointer mInterp;

    Smoother::Pointer mSmoother;

    QList<PropertiesOfPlot> mPlotProps;

    void addPlot(QString descr, const DoubleVector& x, const DoubleVector& y);

    void connectEverything();

    /**
     * @brief choosePlotIdx calls dialog for plot idx choosing
     * @return
     */
    int choosePlotIdx();

    static inline QVector<QCPGraphData> zip(const DoubleVector& x, const DoubleVector& y);
    static inline QPair<DoubleVector, DoubleVector> unzip(const QCPGraphDataContainer& xy);
};

QVector<QCPGraphData> DataPlot::zip
(
    const DataPlot::DoubleVector &x,
    const DataPlot::DoubleVector &y
)
{
    QVector<QCPGraphData> xy(x.size());
    for(int i = 0; i < xy.size(); ++i)
    {
        xy[i].key = x[i];
        xy[i].value = y[i];
    }
    return xy;
}

QPair<DataPlot::DoubleVector, DataPlot::DoubleVector> DataPlot::unzip(const QCPGraphDataContainer &xy)
{
    QPair<DoubleVector, DoubleVector> res;
    res.first.resize(xy.size());
    res.second.resize(xy.size());
    for(int i = 0; i < xy.size(); ++i)
    {
        res.first[i] = xy.at(i)->key;
        res.second[i] = xy.at(i)->value;
    }
    return res;
}

#endif // DATAPLOT_H
