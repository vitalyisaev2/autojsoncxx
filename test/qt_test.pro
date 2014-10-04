#-------------------------------------------------
#
# Project created by QtCreator 2014-10-04T18:53:47
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = qt_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    qt_test.cpp
INCLUDEPATH += ../include ../catch/single_include ../rapidjson/include

CONFIG += c++11
