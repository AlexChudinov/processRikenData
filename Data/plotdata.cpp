#include "plotdata.h"
#include <algorithm>

PlotData::PlotData()
{

}

PlotData::~PlotData()
{

}

void PlotData::addData(const QString &descr, const DataVector &data)
{
    MapStringVector::iterator it = mData.find(descr);
    QString newDescr;
    if(it != mData.end())
    {
       QStringList words = descr.split('#');
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
    mData.insert(newDescr, data);
}

void PlotData::removeData(const QString &descr)
{

}

PlotData::MapStringSize PlotData::dataList() const
{
    return MapStringSize();
}
