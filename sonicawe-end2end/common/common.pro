#-------------------------------------------------
#
# Project created by Johan 2011-12-09
#
#-------------------------------------------------

####################
# Build settings

qtfeatures = ../../../../qtfeatures/
win32:qtfeatures = ..\\\\..\\\\..\\\\..\\\\qtfeatures\\\\

CONFIG += console
CONFIG += $${qtfeatures}sawetestlib

QT += testlib

CONFIG += staticlib
	
TEMPLATE = lib
win32:TEMPLATE = vclib


####################
# Source code

SOURCES += *.cpp
HEADERS += *.h
