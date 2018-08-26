#ifndef PLOTFORM_H
#define PLOTFORM_H

#include <QWidget>
#include "QCustomPlot/qcustomplot.h"

namespace Ui {
class PlotForm;
}

class QCustomPlot;
class CompressedMS;
class QToolBar;

class PlotForm : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief PlotForm creates plot of mass spectrum
     * @param ms mass spectrum
     * @param strDscrpt short data description
     * @param parent
     */
    explicit PlotForm(const CompressedMS &ms, const QString &strDscrpt, QWidget *parent = 0);
    ~PlotForm();

private slots:
    void on_actionZoomOut_triggered();

    void on_actionImport_triggered();

    void on_actionSplineSmoothing_triggered();

private:
    Q_SLOT void on_actionHorizontalZoom_triggered();

    Ui::PlotForm *ui;

    QCustomPlot * m_pPlot;

    //Data
    QScopedPointer<CompressedMS> m_pMassSpec;
    QScopedPointer<CompressedMS> m_pSmoothedData;
    //Limits
    size_t m_nXMin, m_nXMax;
    size_t m_nYMin, m_nYMax;

    /**
     * @brief addMassSpecGraph plots current mass spec
     */
    void addMassSpecGraph();

    /**
     * @brief addSmoothedGraph adds plot obtained
     * using after smoothing procedure
     */
    void addSmoothedGraph();

    /**
     * @brief adjustRangeToLimits puts restriction on zooming
     */
    Q_SLOT void adjustRangeToLimits(QCPRange);

    /**
     * @brief msg sends textual message to user
     * @param msg
     */
    void msg(const QString& msg);

    void setUpToolBar(QToolBar *toolBar);

    void addCompressedDataToGraph(QCPGraph* g, const CompressedMS* ms) const;
};

#endif // PLOTFORM_H
