#-------------------------------------------------
#
# Project created by Johan 2011-12-09
#
#-------------------------------------------------

####################
# Build settings

qtfeatures = ../../../../qtfeatures/
win32:qtfeatures = "..\\..\\..\\..\\qtfeatures\\"

CONFIG += $${qtfeatures}indietest
CONFIG += $${qtfeatures}sawetestlib


####################
# Source code

SOURCES += *.cpp
