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
    Math/CurveFitting.cpp \
    Math/alglib/alglibinternal.cpp \
    Math/alglib/alglibmisc.cpp \
    Math/alglib/ap.cpp \
    Math/alglib/dataanalysis.cpp \
    Math/alglib/diffequations.cpp \
    Math/alglib/fasttransforms.cpp \
    Math/alglib/integration.cpp \
    Math/alglib/interpolation.cpp \
    Math/alglib/linalg.cpp \
    Math/alglib/optimization.cpp \
    Math/alglib/solvers.cpp \
    Math/alglib/specialfunctions.cpp \
    Math/alglib/statistics.cpp

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
    Math/CurveFitting.h \
    Math/alglib/alglibinternal.h \
    Math/alglib/alglibmisc.h \
    Math/alglib/ap.h \
    Math/alglib/dataanalysis.h \
    Math/alglib/diffequations.h \
    Math/alglib/fasttransforms.h \
    Math/alglib/integration.h \
    Math/alglib/interpolation.h \
    Math/alglib/linalg.h \
    Math/alglib/optimization.h \
    Math/alglib/solvers.h \
    Math/alglib/specialfunctions.h \
    Math/alglib/statistics.h \
    Math/alglib/stdafx.h

FORMS    += mainwindow.ui \
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
    QCustomPlot/GPL.txt \
    Math/alglib/manual.cpp.html \
    Math/alglib/gpl2.txt \
    Math/alglib/gpl3.txt

if(win32)
{
    INCLUDEPATH += C:/opencv/build/include/
    INCLUDEPATH += C:/opencv/build/include/
    CONFIG(debug, debug|release)
    {
        LIBS += C:\opencv\build\x64\vc15\lib\opencv_world343d.lib
    }
    CONFIG(release, debug|release)
    {
        LIBS += C:\opencv\build\x64\vc15\lib\opencv_world343.lib
    }
}

