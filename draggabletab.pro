#-------------------------------------------------
#
# Project created by QtCreator 2012-10-16T20:21:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = draggabletab
TEMPLATE = app


SOURCES += main.cpp\
    docktabwidget.cpp \
    docktabmotherwidget.cpp

HEADERS  += \
    docktabwidget.h \
    docktabmotherwidget.h

QMAKE_CXXFLAGS += -std=c++11

