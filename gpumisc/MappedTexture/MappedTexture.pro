#-------------------------------------------------
#
# Project created by QtCreator 2011-03-07T20:41:08
#
#-------------------------------------------------

####################
# Build settings

qtfeatures = ../../../../qtfeatures/
win32:qtfeatures = ..\\\\..\\\\..\\\\..\\\\qtfeatures\\\\

CONFIG += $${qtfeatures}indietest


####################
# Source code

SOURCES += *.cpp

CUDA_SOURCES += \
    *.cu

# "Other files" for Qt Creator
OTHER_FILES += \
    $$CUDA_SOURCES \
