QT += core
QT += network
QT -= gui

CONFIG += c++11

TARGET = Server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    Main.cpp \
    Server.cpp

HEADERS += \
    Server.h
