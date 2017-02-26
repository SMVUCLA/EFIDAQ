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
        efidaq.cpp \
    testsetup.cpp \
    tmodels.cpp \
    runtest.cpp \
    utilities.cpp \
    qcustomplot.cpp \
    serialreader.cpp \
    mainruntest.cpp \
    afrtable.cpp \
    plotwindow.cpp \
    qformdialog.cpp \
    udpreader.cpp \
    sender.cpp \
    datainterpreter.cpp \
    signals.cpp

HEADERS  += efidaq.h \
    testsetup.h \
    tmodels.h \
    runtest.h \
    utilities.h \
    qcustomplot.h \
    serialreader.h \
    mainruntest.h \
    afrtable.h \
    wrappingqvector.h \
    plotwindow.h \
    qformdialog.h \
    udpreader.h \
    sender.h \
    datainterpreter.h \
    signals.h

FORMS    += efidaq.ui \
    testsetup.ui \
    runtest.ui \
    mainruntest.ui \
    afrtable.ui \
    plotwindow.ui \
    qformdialog.ui

RESOURCES += \
    efidaq.qrc

DISTFILES += \
    TODO.txt
