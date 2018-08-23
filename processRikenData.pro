#-------------------------------------------------
#
# Project created by QtCreator 2018-08-20T10:41:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = processRikenData
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    RikenData/rawrikendata.cpp \
    QCustomPlot/qcustomplot.cpp \
    Plot/PlotForm.cpp \
    PropertiesListForm.cpp \
    Math/splines.cpp \
    MassSpecAccDialogs.cpp \
    Math/Interpolator.cpp

HEADERS  += mainwindow.h \
    RikenData/rawrikendata.h \
    QCustomPlot/qcustomplot.h \
    Plot/PlotForm.h \
    PropertiesListForm.h \
    Math/exception.h \
    Math/smoothing_splines.h \
    Math/Solvers.h \
    Math/splines.h \
    MassSpecAccDialogs.h \
    Utils/BaseDefs.h \
    Utils/Factory.h \
    Math/Interpolator.h

FORMS    += mainwindow.ui \
    Plot/PlotForm.ui \
    PropertiesListForm.ui \
    SimpleMassSpecAccDialog.ui \
    SimpleMassSpecAccDialog.ui

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -std=c++0x
