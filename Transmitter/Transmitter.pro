#-------------------------------------------------
#
# Project created by QtCreator 2015-11-28T17:29:56
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Transmitter
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    listenthread.cpp \
    sendthread.cpp

HEADERS  += mainwindow.h \
    listenthread.h \
    sendthread.h

FORMS    += mainwindow.ui
