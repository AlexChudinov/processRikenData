#-------------------------------------------------
#
# Project created by QtCreator 2018-08-20T10:41:04
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = processRikenData
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    QCustomPlot/qcustomplot.cpp \
    PropertiesListForm.cpp \
    MassSpecAccDialogs.cpp \
    Math/Interpolator.cpp \
    Math/ParSplineCalc.cpp \
    CreditsDialog.cpp \
    Math/Smoother.cpp \
    Math/LogSplinePoissonWeight.cpp \
    QMapPropsDialog.cpp \
    DialogAbout.cpp \
    Base/BaseObject.cpp \
    Base/ThreadPool.cpp \
    Plot/BasePlot.cpp \
    Data/Reader.cpp \
    Data/TimeEvents.cpp \
    Data/MassSpec.cpp \
    Plot/PlotPair.cpp \
    Data/XValsTransform.cpp \
    Math/MassSpecSummator.cpp \
    Plot/DataPlot.cpp \
    Data/plotdata.cpp \
    PropertiesTableForm.cpp

HEADERS  += mainwindow.h \
    QCustomPlot/qcustomplot.h \
    PropertiesListForm.h \
    Math/exception.h \
    Math/Solvers.h \
    MassSpecAccDialogs.h \
    Math/ParSplineCalc.h \
    CreditsDialog.h \
    Math/Smoother.h \
    Math/LogSplinePoissonWeight.h \
    QMapPropsDialog.h \
    DialogAbout.h \
    Base/BaseObject.h \
    Base/ThreadPool.h \
    Plot/BasePlot.h \
    Data/Reader.h \
    Data/TimeEvents.h \
    Data/MassSpec.h \
    Plot/PlotPair.h \
    Data/XValsTransform.h \
    Math/MassSpecSummator.h \
    Plot/DataPlot.h \
    Math/interpolator.h \
    Data/plotdata.h \
    PropertiesTableForm.h

FORMS    += mainwindow.ui \
    PropertiesListForm.ui \
    SimpleMassSpecAccDialog.ui \
    AccumScaleCorrectionDialog.ui \
    CreditsDialog.ui \
    QMapPropsDialog.ui \
    DialogAbout.ui \
    PropertiesTableForm.ui

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -std=c++0x

DISTFILES += \
    QCustomPlot/GPL.txt
