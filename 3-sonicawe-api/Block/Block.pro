#-------------------------------------------------
#
# Project created by QtCreator 2011-03-07T21:17:06
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = Block
CONFIG   += console

TEMPLATE = app
win32:TEMPLATE = vcapp


SOURCES += tst_blocktest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
