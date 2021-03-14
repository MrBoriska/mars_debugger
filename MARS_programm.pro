#-------------------------------------------------
#
# Project created by QtCreator 2017-07-03T22:34:05
#
#-------------------------------------------------

QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

TARGET = MARS_programm
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    modelconfig.cpp \
    paintscene.cpp \
    itemeditor.cpp \
    paintpolygonitem.cpp \
    pathplannerservice.cpp \
    unititem.cpp \
    paintpoint.cpp \
    objectitem.cpp \
    modeloptionseditor.cpp \
    trackitem.cpp \
    paintitem.cpp \
    qcustomplot/qcustomplot.cpp \
    modelanalisisviewer.cpp \
    paintview.cpp \
    controlsysservice.cpp

HEADERS  += mainwindow.h \
    modelconfig.h \
    paintscene.h \
    itemeditor.h \
    paintpolygonitem.h \
    pathplannerservice.h \
    unititem.h \
    paintpoint.h \
    objectitem.h \
    modeloptionseditor.h \
    trackitem.h \
    paintitem.h \
    qcustomplot/qcustomplot.h \
    modelanalisisviewer.h \
    paintview.h \
    controlsysservice.h

FORMS    += mainwindow.ui \
    itemeditor.ui \
    modeloptionseditor.ui \
    modelanalisisviewer.ui

RESOURCES += \
    files.qrc
