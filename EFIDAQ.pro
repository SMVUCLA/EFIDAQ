#-------------------------------------------------
#
# Project created by QtCreator 2016-11-15T16:54:29
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = EFIDAQ
TEMPLATE = app


SOURCES += main.cpp\
    tmodels.cpp \
    utilities.cpp \
    qcustomplot.cpp \
    afrtable.cpp \
    plotwindow.cpp \
    qformdialog.cpp \
    datainterpreter.cpp \
    signals.cpp \
    daqwindow.cpp \
    serialhandler.cpp \
    listmodel.cpp

HEADERS  += \
    tmodels.h \
    utilities.h \
    qcustomplot.h \
    afrtable.h \
    wrappingqvector.h \
    plotwindow.h \
    qformdialog.h \
    datainterpreter.h \
    signals.h \
    daqwindow.h \
    serialhandler.h \
    listmodel.h

FORMS    += \
    afrtable.ui \
    plotwindow.ui \
    qformdialog.ui \
    daqwindow.ui

RESOURCES += \
    efidaq.qrc

DISTFILES += \
    TODO.txt
