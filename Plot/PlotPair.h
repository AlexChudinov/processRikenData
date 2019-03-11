#ifndef PLOTPAIR_H
#define PLOTPAIR_H

#include <QMainWindow>
#include "BasePlot.h"

class XValsTransform;

/**
 * @brief The PlotPair class window contains tic and ms plot both
 */

class PlotPair : public QMainWindow
{
    Q_OBJECT
public:
    explicit PlotPair(QWidget *parent = Q_NULLPTR);

    virtual ~PlotPair();

signals:
    /**
     * @brief dataSelected data for data plot were selected
     * @param x x-values
     * @param y y-values
     * @param dscr data description that will be shown in the DataPlot capture
     */
    void dataSelected(QVector<double> x, QVector<double> y, QString dscr);

public slots:
    /**
     * @brief setTicCursorPos sets vertical red line at TIC graph
     * and plot correspondent mass spectrum at MS plot
     * @param x new position of vertical red line
     */
    void setTicCursorPos(double x);
    void setTicCursorPos(QMouseEvent * evt);

private slots:
    /**
     * @brief clearData clear plot data
     */
    void clearData();

    /**
     * @brief massSpecNumsChanged update plots when mass specs number is changed
     */
    void massSpecNumsChanged(size_t massSpecNum);

    void onExportImage();

    void onShowMs();

    void onSelectData();

    void selectMsData();

    void selectTicData();
private:
    QPointer<BasePlot> mMsPlot;

    QPointer<BasePlot> mTicPlot;

    XValsTransform * mXValsTransform;

    void keyPressEvent(QKeyEvent * evt);

    void createGraphs();

    void connectActions();

    void connectPlots();
};

#endif // PLOTPAIR_H
