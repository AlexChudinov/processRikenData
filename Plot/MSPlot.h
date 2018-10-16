#ifndef MSPLOT_H
#define MSPLOT_H

#include <QPointer>
#include <QMainWindow>
#include <mutex>

class BasePlot;

class MSPlot : public QMainWindow
{
    Q_OBJECT

public:
    MSPlot(QWidget * parent = Q_NULLPTR);

    Q_SLOT void plotMS();

    /**
     * @brief setLimits change limits for mass spectrum calculation
     */
     Q_SLOT void setLimits(size_t first, size_t last);

    /**
     * @brief updateLast shows last mass spectrum
     */
    Q_SLOT void updateLast();
private:
    QPointer<BasePlot> mPlot;
    size_t mFirst;
    size_t mLast;
};

#endif // MSPLOT_H
