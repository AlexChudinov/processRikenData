#ifndef MSPLOT_H
#define MSPLOT_H

#include <QPointer>
#include <QMainWindow>
#include <mutex>
#include "BasePlot.h"

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
    Q_SLOT void updateLast(size_t msCount);

    /**
     * @brief showMassSpec shows mass spec with particular number
     * @param num
     */
    Q_SLOT void showMassSpec(size_t num);
private:
    QPointer<BasePlot> mPlot;
    size_t mFirst;
    size_t mLast;

    template<class MassSpecData>
    void plotMassSpec(const MassSpecData& d);
};

template<class MassSpecData>
void MSPlot::plotMassSpec(const MassSpecData &d)
{
    if(!d.empty())
    {
        QVector<double> vXData, vYData;
        vXData.reserve(static_cast<int>(d.size()) + 2);
        vYData.reserve(vXData.size());
        vXData.push_back(d.begin()->first - 1);
        vYData.push_back(0.0);
        for(typename MassSpecData::const_reference r : d)
        {
            if(*vXData.rbegin() + 1 < r.first)
            {
                if(r.first - 1 > *vXData.rbegin() + 1)
                {
                    vXData.push_back(*vXData.rbegin() + 1);
                    vYData.push_back(0.0);
                }
                vXData.push_back(r.first - 1);
                vYData.push_back(0.0);
            }
            vXData.push_back(r.first);
            vYData.push_back(r.second);
        }
        mPlot->graph(0)->setData(vXData, vYData);
        mPlot->replot();
        mPlot->rescaleAxes();
    }
}

#endif // MSPLOT_H
