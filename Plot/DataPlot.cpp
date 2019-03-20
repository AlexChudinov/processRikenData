#include <QDialog>
#include <exception>

#include "../QMapPropsDialog.h"
#include "Base/BaseObject.h"
#include "DataPlot.h"
#include "BasePlot.h"
#include "Math/LogSplinePoissonWeight.h"

const QList<Qt::GlobalColor> DataPlot::s_colors
{
    Qt::black,
    Qt::darkGray,
    Qt::red,
    Qt::green,
    Qt::blue,
    Qt::cyan,
    Qt::magenta,
    Qt::darkRed,
    Qt::darkGreen,
    Qt::darkBlue,
    Qt::darkCyan,
    Qt::darkMagenta,
    Qt::darkYellow
};

DataPlot::DataPlot
(
    const DoubleVector &x,
    const DoubleVector &y,
    const QString& capture,
    const QString &xLabel,
    const QString &yLabel,
    QWidget *parent
)
    :
      QMainWindow(parent),
      mPlot(new BasePlot(this)),
      mInterp(Interpolator::create("Linear"))
{
    mPlot->setWindowTitle(capture);
    setCentralWidget(mPlot);
    mPlot->toolBar()->addAction
    (
        QIcon("://Icons//splineSmoothing"),
        tr("Smooth data with spline"),
        this,
        SLOT(calculateSmoothing())
    );
    mPlot->toolBar()->addAction
    (
        QIcon("://Icons//interp"),
        "Choose interpolator",
        this,
        SLOT(chooseInterpolator())
    );
    mPlot->toolBar()->addAction
    (
        QIcon("://Icons//props"),
        tr("Plot properties"),
        this,
        SLOT(on_showProps())
    );
    addToolBar(mPlot->toolBar());
    setStatusBar(new QStatusBar);

    mPlot->xAxis->setLabel(xLabel);
    if(!yLabel.isEmpty()) mPlot->xAxis->setLabel(yLabel);

    addPlot("Raw data",x, y);

    mPlot->rescaleAxes();
    mPlot->replot();

    connectEverything();
}

void DataPlot::calculateSmoothing()
{
    try {
        createSmoother();
        if(mSmoother)
        {
            const QCPRange range = mPlot->xAxis->range();

            const PropertiesOfPlot& props = mPlotProps[choosePlotIdx()];

            QCPGraphDataContainer::const_iterator _First = props.mData->findBegin(range.lower);
            QCPGraphDataContainer::const_iterator _Last = props.mData->findEnd(range.upper);

            //Early return if the data otside the range
            if(_First == std::prev(props.mData->end())) return;

            const size_t n = static_cast<size_t>(std::distance(_First, _Last));
            Interpolator::Vector x(n), y(n);
            for(size_t i = 0; _First != _Last; ++_First, ++i)
            {
                x[i] = _First->key;
                y[i] = _First->value;
            }

            double step;
            Interpolator::Vector yNew = mInterp->equalStepData(x, y, step),
                    xNew(yNew.size()),
                    ySmoothed;

            double xx0 = x[0] - step;
            for(double & xx : xNew) xx = xx0 += step;

            std::replace_if
            (
                yNew.begin(),
                yNew.end(),
                [](double val)->bool { return val < 0.; },
                0.
            );

            mSmoother->run(ySmoothed, yNew);

            addPlot
            (
                tr("Smoothed data mz: %1 - %2").arg(x.front()).arg(x.back()),
                 QVector<double>::fromStdVector(xNew),
                 QVector<double>::fromStdVector(ySmoothed)
            );
        }
    }
    catch (const std::exception& e)
    {
        QString except = tr("An exception \"") + e.what() + "\" catched!";
        QMessageBox::warning(this, "Data plot message", except);
    }

}

void DataPlot::createSmoother()
{
    bool ok;
    QString item = QInputDialog::getItem
    (
        this,
        tr("Smoother type"),
        tr("Choose smoother type: "),
        Smoother::types(),
        0,
        true,
        &ok
    );

    if(ok)
    {
        mSmoother.reset(Smoother::create(item).release());

        QMapPropsDialog dialog(this);
        dialog.setProps(mSmoother->params());
        dialog.exec();
        mSmoother->setParams(dialog.props());
    }
}

void DataPlot::chooseInterpolator()
{
    QString item = QInputDialog::getItem
    (
        this,
        tr("Set interpolator"),
        tr("Interpolator type"),
        Interpolator::names()
    );
    mInterp.reset(Interpolator::create(item).release());
}

void DataPlot::onShowMs()
{
    int cnt = 0;
    mPlot->clearGraphs();
    for(const auto& d : mPlotProps)
    {
        mPlot->addGraph(QPen(d.mColor, d.mLineWidth));
        QCPGraph * g = mPlot->graph(cnt++);
        g->setData(d.mData);
    }
    mPlot->replot();
}

void DataPlot::on_showProps()
{
    PropertiesOfPlotForm props(this);
    props.addProps(mPlotProps);
    props.exec();
    if(props.result() == QDialog::Accepted)
    {
        props.readProps(mPlotProps);
        onShowMs();
    }
}

void DataPlot::addPlot(QString descr, const DoubleVector &x, const DoubleVector &y)
{
    static int colorIdx;
    colorIdx = (colorIdx + 1) % s_colors.size();
    mPlotProps.push_back
    (
        {
            QColor(s_colors[colorIdx]),
            3,
            descr,
            QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer)
        }
    );
    mPlotProps.back().mData->set(zip(x,y));
    onShowMs();
}

void DataPlot::connectEverything()
{
    connect
    (
        mPlot.data(),
        SIGNAL(mouseCoordinateNotify(QString)),
        statusBar(),
        SLOT(showMessage(QString))
    );
}

int DataPlot::choosePlotIdx()
{
    QStringList items;
    for(const auto& i : mPlotProps)
    {
        items.push_back(i.mDescription);
    }
    QString item = QInputDialog::getItem(this, tr("Choose plot"), tr("Plots list"), items);
    return std::distance(items.begin(), std::find(items.begin(), items.end(), item));
}

PropertiesOfPlotForm::PropertiesOfPlotForm(QWidget *parent)
    :
      QDialog(parent),
      verticalLayout(new QVBoxLayout),
      buttonBox(new QDialogButtonBox)
{
    setWindowModality(Qt::WindowModal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    verticalLayout->addWidget(buttonBox);
    setLayout(verticalLayout);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

PropertiesOfPlotForm::~PropertiesOfPlotForm()
{

}

void PropertiesOfPlotForm::addProps(const QList<PropertiesOfPlot> &props)
{
    int propsIdx = 0;
    for(const auto& prop : props) addPropsEntry(propsIdx++, prop);
    adjustSize();
    update();
}

void PropertiesOfPlotForm::readProps(QList<PropertiesOfPlot> &props)
{
    QList<PropertiesOfPlot> newProps;
    for(auto it = mWidgets.begin(); it != mWidgets.end(); ++it)
    {
        PropertiesOfPlot p;
        p.mDescription = it.value().mName->text();
        p.mLineWidth = it.value().mLineWidth->value();
        p.mColor = it.value().mColor->palette().button().color();
        p.mData = props[it.key()].mData;
        newProps.push_back(p);
    }
    props = newProps;
}

void PropertiesOfPlotForm::chooseColor()
{
    QPushButton * btn = qobject_cast<QPushButton*>(QObject::sender());
    QColor color;
    color = QColorDialog::getColor(btn->palette().button().color());
    if(color.isValid())
    {
        btn->setStyleSheet(QString("background-color: " + color.name() + ";"));
    }
}

void PropertiesOfPlotForm::deletePlot()
{
    QPushButton * btn = qobject_cast<QPushButton*>(QObject::sender());
    auto it = std::find_if
    (
        mWidgets.begin(),
        mWidgets.end(),
        [&](const PropsWidgets& w)->bool
        {
            return w.mDelete == btn;
        }
    );
    it.value().mName->deleteLater();
    it.value().mLineWidth->deleteLater();
    it.value().mColor->deleteLater();
    it.value().mDelete->deleteLater();
    it.value().mSave->deleteLater();
    mWidgets.erase(it);
    update();
}

void PropertiesOfPlotForm::saveToAscii()
{
    QPushButton * btn = qobject_cast<QPushButton*>(QObject::sender());
    auto it = std::find_if
    (
        mWidgets.begin(),
        mWidgets.end(),
        [&](const PropsWidgets& w)->bool
        {
            return w.mSave == btn;
        }
    );
    DataPlot * plot = qobject_cast<DataPlot*>(parent());
    QCPRange xRange = plot->mPlot->xAxis->range();
    auto
        _First = plot->mPlotProps[it.key()].mData->findBegin(xRange.lower),
        _Last = plot->mPlotProps[it.key()].mData->findEnd(xRange.upper);

    QString fileName = QFileDialog::getSaveFileName
    (
        this,
        tr("Export data in a text file"),
        QString(),
        tr("Text files (*.txt)")
    );
    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        file.open(QIODevice::Text | QIODevice::WriteOnly);
        QTextStream stream(&file);
        //x-values should be different
        stream.setRealNumberPrecision
        (
            decimals
            (
                std::prev(_Last)->key,
                std::prev(std::prev(_Last))->key
            )
        );
        for(; _First != _Last; ++_First)
        {
            stream << _First->key << "\t" << _First->value << "\n";
        }
        file.close();
    }
}

void PropertiesOfPlotForm::addPropsEntry(int idx, const PropertiesOfPlot &property)
{
    QHBoxLayout * layout = new QHBoxLayout;

    QLineEdit * name = new QLineEdit(property.mDescription);
    layout->addWidget(name);

    QSpinBox * lineWidth = new QSpinBox;
    lineWidth->setValue(property.mLineWidth);
    lineWidth->setRange(0, 100);
    layout->addWidget(lineWidth);

    QPushButton * btn = new QPushButton("color");
    btn->setStyleSheet(QString("background-color: " + property.mColor.name() + ";"));
    layout->addWidget(btn);

    QPushButton * btnDel = new QPushButton(QIcon("://Icons//del"), tr("delete"));
    layout->addWidget(btnDel);

    QPushButton * btnSave = new QPushButton(QIcon("://Icons//importTextData"), tr("save ascii"));
    layout->addWidget(btnSave);

    connect(btn, SIGNAL(pressed()), this, SLOT(chooseColor()));

    connect(btnDel, SIGNAL(pressed()), this, SLOT(deletePlot()));

    connect(btnSave, SIGNAL(pressed()), this, SLOT(saveToAscii()));

    verticalLayout->insertLayout(0, layout);
    mWidgets[idx] = PropsWidgets{name, lineWidth, btn, btnDel, btnSave};
}
