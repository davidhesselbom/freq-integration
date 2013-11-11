#-------------------------------------------------
#
# Project created by QtCreator 2011-03-07T20:41:08
#
#-------------------------------------------------

####################
# Build settings

CONFIG += console
CONFIG += c++11 buildflags
CONFIG += cuda
CONFIG += sawelibs

QT -= core
QT -= gui

TEMPLATE = app
win32:TEMPLATE = vcapp


####################
# Source code

!usecuda:SOURCES += *.cpp

CUDA_SOURCES += \
    *.cu

# "Other files" for Qt Creator
OTHER_FILES += \
    $$CUDA_SOURCES \