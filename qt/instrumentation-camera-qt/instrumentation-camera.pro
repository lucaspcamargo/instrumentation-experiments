#-------------------------------------------------
#
# Project created by QtCreator 2015-12-02T17:15:51
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = instrumentation-camera
TEMPLATE = app


SOURCES += main.cpp\
        serialcamerawindow.cpp \
    qdserialport.cpp \
    serialcamera.cpp

HEADERS  += serialcamerawindow.h \
    qdserialport.h \
    serialcamera.h

FORMS    += serialcamerawindow.ui

RESOURCES += \
    res.qrc
