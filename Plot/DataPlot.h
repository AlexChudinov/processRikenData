#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QMainWindow>
#include <QPointer>

class BasePlot;

class DataPlot : public QMainWindow
{
    Q_OBJECT
public:
    explicit DataPlot
    (
        const QVector<double>& x,
        const QVector<double>& y,
        QWidget *parent = Q_NULLPTR
    );

signals:

public slots:

private:
    QPointer<BasePlot> mPlot;

};

#endif // DATAPLOT_H
