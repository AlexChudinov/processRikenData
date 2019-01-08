#ifndef PLOTPAIR_H
#define PLOTPAIR_H

#include <QMainWindow>
#include "BasePlot.h"

/**
 * @brief The PlotPair class window contains tic and ms plot both
 */

class PlotPair : public QMainWindow
{
    Q_OBJECT
public:
    explicit PlotPair(QWidget *parent = 0);

signals:

public slots:

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

private:
    QPointer<BasePlot> mMsPlot;

    QPointer<BasePlot> mTicPlot;

    void createGraphs();

    void connectActions();

    void connectPlots();
};

#endif // PLOTPAIR_H
