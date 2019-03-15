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
    using MapStringVector = QMap<QString, DataVector>;
    using MapStringSize = QMap<QString, DataVector::size_type>;

    PlotData();
    virtual ~PlotData();

    /**
     * @brief adding / removing data from the list
     */
    virtual void addData(const QString& descr, const DataVector& data);
    virtual void removeData(const QString& descr);

    virtual MapStringSize dataList() const;
private:
    MapStringVector mData;
};

#endif // PLOTDATA_H
