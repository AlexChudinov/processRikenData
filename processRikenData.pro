#-------------------------------------------------
#
# Project created by QtCreator 2018-08-20T10:41:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = processRikenData
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    RikenData/rawrikendata.cpp \
    rikendataheaderform.cpp \
    simplemassspecaccdialog.cpp \
    QCustomPlot/qcustomplot.cpp

HEADERS  += mainwindow.h \
    RikenData/rawrikendata.h \
    rikendataheaderform.h \
    simplemassspecaccdialog.h \
    QCustomPlot/qcustomplot.h

FORMS    += mainwindow.ui \
    rikendataheaderform.ui \
    simplemassspecaccdialog.ui

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -std=c++0x
