#ifndef MSPLOT_H
#define MSPLOT_H

#include "BasePlot.h"

class MSPlot : public BasePlot
{
    Q_OBJECT

public:
    MSPlot(QWidget * parent = Q_NULLPTR);

    Q_SLOT void plotMS(size_t idx);
private:
    size_t mFirstMassIdx;
    size_t mLastMassIdx;
    size_t mFirstTimeIdx;
    size_t mLastTimeIdx;

    QCPGraph * mMSGraph;
};

#endif // MSPLOT_H
