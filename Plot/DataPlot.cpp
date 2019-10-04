#include <QDialog>
#include <exception>

#include "Math/CurveFitting.h"
#include "Math/alglib/interpolation.h"
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
        QIcon("://Icons/importTextData"),
        tr("Extract text data from plot"),
        this,
        SLOT(on_importTxt())
    );
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
    mPlot->toolBar()->addAction
    (
        QIcon("://Icons//fitData"),
        tr("Fit data inside window"),
        this,
        SLOT(on_fitData())
    );
    mPlot->toolBar()->addAction
    (
        QIcon("://Icons/extractShape"),
        tr("Extract peak shape inside window"),
        this,
        SLOT(on_createPeakShape())
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

            StdDoubleVector ySmoothed, y, x;
            equalRangedDataPoints(x, y, choosePlotIdx());
            std::replace_if
            (
                y.begin(),
                y.end(),
                [](double val)->bool { return val < 0.; },
                0.
            );

            mSmoother->run(ySmoothed, y);

            addPlot
            (
                tr("Smoothed data mz: %1 - %2").arg(x.front()).arg(x.back()),
                 QVector<double>::fromStdVector(x),
                 QVector<double>::fromStdVector(ySmoothed)
            );

            double s = 0.0;
            int cnt = 0;
            for(size_t i = 0; i < ySmoothed.size(); ++i)
            {
                if(ySmoothed[i] >= 1.)
                {
                    const double ds = ySmoothed[i] - y[i];
                    s += ds * ds / ySmoothed[i];
                    cnt++;
                }
            }

            showInfoMessage
            (
                tr("Chi-square: %1").arg(s / cnt)
            );
            if(mSmoother->type() == Smoother::LogSplineFixNoiseValue)
            {
                LSFixNoiseValue * pSpline = dynamic_cast<LSFixNoiseValue *>(mSmoother.get());
                PeakParams * peakParams = dynamic_cast<PeakParams *>(pSpline);
                QLocale locale;
                QString strPos = locale.toString
                (
                    peakParams->peakPosition() + * x.begin(),
                    'f',
                    10
                );
                showInfoMessage
                (
                    tr("Peak position: %1 +/- %2")
                            .arg(strPos)
                            .arg(peakParams->peakPositionUncertainty())
                );
            }
            if(mSmoother->type() == Smoother::AlglibSplineType)
            {
                showInfoMessage
                (
                    tr("Number of spline points: %1\n")
                            .arg
                            (
                                mSmoother->params()["Spline nodes:"].toInt()
                            )
                );
            }
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
    else
    {
        mSmoother.release();
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

void DataPlot::on_fitData()
{
    bool ok;
    QStringList items = CurveFitting::implementations();
    QString item = QInputDialog::getItem
    (
        this,
        tr("Choose approximator"),
        tr("Available approximators"),
        (items << "Shape fit" << "Double shape fit" << "Multi-shape fit"),
        0,
        true,
        &ok
    );
    if(ok)
    {
        if(item == "Shape fit") on_fitPeakShape();
        else if (item == "Double shape fit") on_fitDoublePeakShape();
        else if (item == "Multi-shape fit") on_fitMultiShape();
        else
        {
            StdDoubleVector x, y;

            equalRangedDataPoints(x, y, choosePlotIdx());

            CurveFitting::Ptr approx = CurveFitting::create(item, x, y);

            StdDoubleVector yy;
            approx->values(x, yy);

            addPlot
            (
                tr("Data fit in range %1 - %2").arg(x.front()).arg(x.back()),
                DoubleVector::fromStdVector(x),
                DoubleVector::fromStdVector(yy)
            );

            double s = 0.0;
            int cnt = 0;
            for(size_t i = 0; i < yy.size(); ++i)
            {
                if(yy[i] > 0.)
                {
                    const double ds = yy[i] - y[i];
                    s += ds * ds / yy[i];
                    cnt++;
                }
            }

            showInfoMessage
            (
                tr
                (
                    "Chi-square: %1\n"
                    "Peak position: %2\n"
                    "Peak position uncertainty: %3\n"
                )
                        .arg(s / cnt)
                        .arg(approx->peakPosition(), 0, 'g', 10)
                        .arg(approx->peakPositionUncertainty(), 0, 'g', 3)
            );
        }
    }
}

void DataPlot::on_createPeakShape()
{
    int nPlot = choosePlotIdx();

    if(nPlot != 0)
    {
        //equalRangedDataPoints(x, y, nPlot);
        QSharedPointer<QCPGraphDataContainer> data
                = mPlot->graph(nPlot)->data();
        StdDoubleVector x(data->size()), y(data->size());
        QCPGraphDataContainer::const_iterator it = data->begin();
        for(size_t i = 0; i < x.size(); ++i, ++it)
        {
            x[i] = it->key;
            y[i] = it->value;
        }
        mPeakShape.reset(new PeakShapeFit(x, y));
        showInfoMessage
        (
            tr
            (
                "Peak position: %1\n"
                "Peak position uncertainty: %2\n"
            )
                    .arg(mPeakShape->peakPosition(), 0, 'g', 10)
                    .arg(mPeakShape->peakPositionUncertainty(), 0, 'g', 3)
        );
    }
}

void DataPlot::on_fitPeakShape()
{
    if(mPeakShape)
    {
        StdDoubleVector x, y;

        equalRangedDataPoints(x, y, choosePlotIdx());

        mPeakShape->fit(x, y);

        showInfoMessage
        (
            tr
            (
                "PeakPosition: %1\n"
                "Peak uncertainty: %2\n"
            )
                    .arg(mPeakShape->peakPosition(), 0, 'g', 10)
                    .arg(mPeakShape->peakPositionUncertainty(), 0, 'g', 3)
        );

        mPeakShape->values(x, y);

        addPlot
        (
            tr("Shape fit in range %1 - %2").arg(x.front()).arg(x.back()),
            DoubleVector::fromStdVector(x),
            DoubleVector::fromStdVector(y)
        );
    }
}

void DataPlot::on_fitDoublePeakShape()
{
    if(mPeakShape)
    {
        StdDoubleVector x, y;
        equalRangedDataPoints(x, y, choosePlotIdx());

        DoublePeakShapeFit fit(*mPeakShape, x, y);
        StdDoubleVector yy;
        fit.values(x, yy);

        addPlot
        (
            tr("Shape fit in range %1 - %2").arg(x.front()).arg(x.back()),
            DoubleVector::fromStdVector(x),
            DoubleVector::fromStdVector(yy)
        );


        while
        (
            QMessageBox::question
            (
                this,
                tr("Double peak fittig"),
                tr
                (
                    "Peak position1: %1\n"
                    "Peak uncertainty1: %2\n"
                    "Peak position2: %3 \n"
                    "Peak uncertainty2: %4\n"
                    "Do you want to make another run?"
                )
                    .arg(fit.peakPosition1(), 0, 'g', 10)
                    .arg(fit.peakPositionUncertainty1(), 0, 'g', 3)
                    .arg(fit.peakPosition2(), 0, 'g', 10)
                    .arg(fit.peakPositionUncertainty2(), 0, 'g', 3)
            ) == QMessageBox::Yes
        )
        {
            fit.fit(x, y);
            fit.values(x, yy);
            addPlot
            (
                tr("Shape fit in range %1 - %2").arg(x.front()).arg(x.back()),
                DoubleVector::fromStdVector(x),
                DoubleVector::fromStdVector(yy)
            );
        }
    }
}

void DataPlot::on_fitMultiShape()
{
    if(mPeakShape)
    {
        StdDoubleVector x, y;
        equalRangedDataPoints(x, y, choosePlotIdx());

        int nShapes = QInputDialog::getInt(this, "Multifit", "Number of peaks", 2, 1, 20);

        MultiShapeFit fit(*mPeakShape, x, y, nShapes);
        StdDoubleVector yy;
        fit.values(x, yy);

        addPlot
        (
            tr("Shape fit in range %1 - %2").arg(x.front()).arg(x.back()),
            DoubleVector::fromStdVector(x),
            DoubleVector::fromStdVector(yy)
        );

        QString str;
        QTextStream data(&str);
        fit.importData(data);

        while(QInputDialog::getMultiLineText
        (
            this,
            tr("Multifit"),
            tr("Fit result:"),
            str
        ).isNull())
        {
            fit.fit(x,y);
            str.clear();
            fit.importData(data);
            fit.values(x, yy);
            addPlot
            (
                tr("Shape fit in range %1 - %2").arg(x.front()).arg(x.back()),
                DoubleVector::fromStdVector(x),
                DoubleVector::fromStdVector(yy)
            );
        }
    }
}

void DataPlot::on_importTxt()
{
    QString fileName = QFileDialog::getSaveFileName
    (
        this,
        tr("Save text data"),
        QString()
    );
    if(!fileName.isEmpty() && mPeakShape)
    {
        QFile file(fileName);
        file.open(QFile::WriteOnly);
        QTextStream stream(&file);
        mPeakShape->import(stream);
        file.close();
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

void DataPlot::showInfoMessage(const QString &info) const
{
    QMessageBox wnd
    (
        QMessageBox::Icon::Information,
        "Data plot",
        info
    );

    wnd.exec();
}

void DataPlot::equalRangedDataPoints(StdDoubleVector &x, StdDoubleVector &y, int idx)
{
    PropertiesOfPlot::GraphData data = mPlotProps[idx].mData;
    QCPRange range = mPlot->xAxis->range();
    QCPGraphDataContainer::const_iterator _First = data->findBegin(range.lower);
    QCPGraphDataContainer::const_iterator _Last = data->findEnd(range.upper);
    const size_t n = static_cast<size_t>(std::distance(_First, _Last));
    StdDoubleVector xPlot(n), yPlot(n);
    for(size_t i = 0; i < n; ++i, ++_First)
    {
        xPlot[i] = _First->key;
        yPlot[i] = _First->value;
    }
    QVariantMap props{{"xmin", xPlot[0]}, {"xmax", xPlot[n-1]}, {"step", 1.}};
    QMapPropsDialog setLims;
    setLims.setProps(props);
    setLims.exec();
    props = setLims.props();
    bool ok = true;
    const double step = props["step"].toDouble(&ok);
    const double xmin = props["xmin"].toDouble(&ok);
    const double xmax = props["xmax"].toDouble(&ok);
    Q_ASSERT(ok);
    x.assign(static_cast<size_t>((xmax - xmin)/step), xmin);
    for(size_t i = 1; i < x.size(); ++i)
    {
        x[i] = x[i-1] + step;
    }
    y = mInterp->interpolate(xPlot, yPlot, x);
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
