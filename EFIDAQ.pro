#-------------------------------------------------
#
# Project created by QtCreator 2016-11-15T16:54:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EFIDAQ
TEMPLATE = app


SOURCES += main.cpp\
        efidaq.cpp \
    testsetup.cpp \
    tmodels.cpp \
    runtest.cpp \
    utilities.cpp

HEADERS  += efidaq.h \
    testsetup.h \
    tmodels.h \
    runtest.h \
    utilities.h

FORMS    += efidaq.ui \
    testsetup.ui \
    runtest.ui

RESOURCES += \
    efidaq.qrc
