#ifndef PLOTFORM_H
#define PLOTFORM_H

#include <QWidget>

#include "RikenData/rawrikendata.h"
#include "QCustomPlot/qcustomplot.h"

namespace Ui {
class PlotForm;
}

class QCustomPlot;
class CompressedMS;
class QToolBar;
class PropertiesListForm;

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

    Q_SIGNAL void mouseCoordNotify(const QString& str);

private:
    Q_SLOT void on_actionMassSpecProps_triggered();
    Q_SLOT void on_actionSavePicture_triggered();
    Q_SLOT void on_actionDragXAxisLim_triggered();
    Q_SLOT void on_actionZoomOut_triggered();
    Q_SLOT void on_actionImport_triggered();
    Q_SLOT void on_actionSplineSmoothing_triggered();
    Q_SLOT void on_actionHorizontalZoom_triggered();
    Q_SLOT void printMouseCoordinates(QMouseEvent * event);
    Q_SLOT void mouseClick(QMouseEvent * event);

    Ui::PlotForm *ui;

    QCustomPlot * m_pPlot;

    //Data
    QScopedPointer<CompressedMS> m_pMassSpec;
    QScopedPointer<CompressedMS> m_pSmoothedData;
    QScopedPointer<Peak::PeakCollection> m_peaks;
    QVariantMap m_smoothProps;
    QString m_smootherName;
    //Limits
    size_t m_nXMin, m_nXMax;
    size_t m_nYMin, m_nYMax;

    //Property of current data
    QPointer<PropertiesListForm> m_props;

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

    void importTextDataToFile(QTextStream &out, const QCPGraphDataContainer *tab) const;

    void fillPropertiesList();
};

#endif // PLOTFORM_H
