#include "plotdata.h"
#include <algorithm>

PlotData::PlotData()
{

}

PlotData::~PlotData()
{

}

void PlotData::addData
(
    const QString &descr,
    const DataVector &xData,
    const DataVector &yData
)
{
    Q_ASSERT(xData.size() == yData.size());
    MapStringVector::iterator it = mData.lowerBound(descr);
    QString newDescr;
    if(it != mData.end() && it.key().contains(descr))
    {
        while(it.key().contains(descr)) ++it; --it;
       QStringList words = it.key().split('#');
       bool ok;
       int n = words.last().toInt(&ok);
       Q_ASSERT(ok);
       QStringList::const_iterator it = words.begin();
       for(; it != std::prev(words.end()); ++it)
       {
           newDescr += *it;
       }
       newDescr += QString("#%1").arg(n + 1);
    }
    else
    {
        newDescr = descr + " #1";
    }
    mData.insert(newDescr, {xData, yData});
}

void PlotData::removeData(const QString &descr)
{
    mData.erase(mData.find(descr));
}

PlotData::MapStringSize PlotData::dataList() const
{
    MapStringSize res;

    for(MapStringVector::const_iterator it = mData.begin(); it != mData.end(); ++it)
        res.insert(it.key(), it.value().first.size());

    return res;
}
