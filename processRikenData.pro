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
    RikenData/rawrikendata.cpp \
    QCustomPlot/qcustomplot.cpp \
    Plot/PlotForm.cpp \
    PropertiesListForm.cpp \
    MassSpecAccDialogs.cpp \
    Math/Interpolator.cpp \
    RikenData/CompressedMS.cpp \
    Math/ParSplineCalc.cpp \
    CreditsDialog.cpp \
    Math/Smoother.cpp \
    Math/LogSplinePoissonWeight.cpp \
    QMapPropsDialog.cpp \
    DialogAbout.cpp \
    RikenData/peak.cpp

HEADERS  += mainwindow.h \
    RikenData/rawrikendata.h \
    QCustomPlot/qcustomplot.h \
    Plot/PlotForm.h \
    PropertiesListForm.h \
    Math/exception.h \
    Math/Solvers.h \
    MassSpecAccDialogs.h \
    Math/Interpolator.h \
    RikenData/CompressedMS.h \
    Math/ParSplineCalc.h \
    CreditsDialog.h \
    Math/Smoother.h \
    Math/LogSplinePoissonWeight.h \
    QMapPropsDialog.h \
    DialogAbout.h \
    RikenData/peak.h

FORMS    += mainwindow.ui \
    Plot/PlotForm.ui \
    PropertiesListForm.ui \
    SimpleMassSpecAccDialog.ui \
    AccumScaleCorrectionDialog.ui \
    CreditsDialog.ui \
    QMapPropsDialog.ui \
    DialogAbout.ui

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -std=c++0x

DISTFILES += \
    QCustomPlot/GPL.txt
