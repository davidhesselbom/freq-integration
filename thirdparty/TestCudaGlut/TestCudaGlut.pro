#-------------------------------------------------
#
# Project created by QtCreator 2011-03-07T20:41:08
#
#-------------------------------------------------

####################
# Build settings

qtfeatures = ../../../../qtfeatures/
win32:qtfeatures = "..\\..\\..\\..\\qtfeatures\\"

CONFIG += console
CONFIG += $${qtfeatures}buildflags
CONFIG += $${qtfeatures}cuda

QT -= core
QT -= gui

TEMPLATE = app
win32:TEMPLATE = vcapp


####################
# Source code

SOURCES += *.cpp

CUDA_SOURCES += \
    *.cu

# "Other files" for Qt Creator
OTHER_FILES += \
    $$CUDA_SOURCES \