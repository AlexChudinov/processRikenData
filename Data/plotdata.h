#ifndef PLOTDATA_H
#define PLOTDATA_H

#include <QVector>
#include <QMap>

/**
 * @brief The PlotData class keeper of the plot data
 */
class PlotData
{
public:

    using DataVector = QVector<double>;
    using MapStringVector = QMap<QString, QPair<DataVector, DataVector>>;
    using MapStringSize = QMap<QString, DataVector::size_type>;
    using Iterator = MapStringVector::iterator;
    using ConstIterator = MapStringVector::const_iterator;

    PlotData();
    virtual ~PlotData();

    /**
     * @brief adding / removing data from the list
     */
    virtual void addData(const QString& descr, const DataVector& xData, const DataVector& yData);
    virtual void removeData(const QString& descr);

    virtual MapStringSize dataList() const;

    Iterator begin() { return mData.begin(); }
    Iterator end() { return mData.end(); }
    ConstIterator begin() const { return mData.begin(); }
    ConstIterator end() const { return mData.end(); }

private:
    MapStringVector mData;
};

#endif // PLOTDATA_H
