#-------------------------------------------------
#
# Project created by QtCreator 2015-11-28T17:31:44
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Network
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    listenthread.cpp

HEADERS  += mainwindow.h \
    listenthread.h

FORMS    += mainwindow.ui
