#-------------------------------------------------
#
# Project created by Johan 2011-12-09
#
#-------------------------------------------------

####################
# Build settings

CONFIG += console
CONFIG += c++11 sawetestlib

QT += testlib

CONFIG += staticlib
	
TEMPLATE = lib
win32:TEMPLATE = vclib


####################
# Source code

SOURCES += *.cpp
HEADERS += *.h
