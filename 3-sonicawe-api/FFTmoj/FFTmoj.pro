#-------------------------------------------------
#
# Project created by QtCreator 2011-03-07T20:41:08
#
#-------------------------------------------------

####################
# Build settings

qtfeatures = ../../../../qtfeatures/
win32:qtfeatures = "..\\..\\..\\..\\qtfeatures\\"

CONFIG += $${qtfeatures}indietest
CONFIG += $${qtfeatures}sawetestlib

DESTDIR = release/cpu

####################
# Source code

SOURCES += *.cpp

SONICAWE = ../../../../src/

# #######################################################################
# OpenCL
# #######################################################################
useopenclnvidia {
DEFINES += USE_OPENCL
OBJECTS_DIR = $${OBJECTS_DIR}openclnvidia/

SOURCES += \
    $$GPUMISC/openclcontext.cpp \
    $$GPUMISC/openclmemorystorage.cpp \
    $$SONICAWE/tfr/clfft/*.cpp

HEADERS += \
    $$GPUMISC/openclcontext.h \
    $$GPUMISC/openclmemorystorage.h \
    $$SONICAWE/tfr/clfft/*.h

macx: LIBS += -framework OpenCL
!macx: LIBS += -lOpenCL

win32 {
    # use OpenCL headers from Cuda Gpu Computing SDK
    INCLUDEPATH += "$(CUDA_INC_PATH)"
    LIBS += -L"$(CUDA_LIB_PATH)"
}

unix:!macx {
    OPENCL_DIR = /usr/local/cuda
    INCLUDEPATH += $$OPENCL_DIR/include
}

macx {
    OPENCL_DIR = /usr/local/cuda
    INCLUDEPATH += $$OPENCL_DIR/include
}
}

useopenclamdapple {
CONFIG += useopencl
DEFINES += USE_OPENCL
OBJECTS_DIR = $${OBJECTS_DIR}openclamdapple/
DESTDIR = release/opencl/amdapple

macx: LIBS += -framework OpenCL
!macx: LIBS += -lOpenCL

win32 {
    # use OpenCL headers from AMD APP Computing SDK
    INCLUDEPATH += \
    "$(AMDAPPSDKROOT)include" \

    LIBS += \
    -L"$(AMDAPPSDKROOT)lib/x86" \

}

unix:!macx {
    OPENCL_DIR = /usr/local/cuda
    INCLUDEPATH += $$OPENCL_DIR/include
}

macx {
    OPENCL_DIR = /usr/local/cuda
    INCLUDEPATH += $$OPENCL_DIR/include
}
}

useopenclamdamd {
DEFINES += USE_OPENCL
DEFINES += USE_AMD
OBJECTS_DIR = $${OBJECTS_DIR}openclamdamd/
DESTDIR = release/opencl/amdamd

macx: LIBS += -framework OpenCL
!macx: LIBS += -lOpenCL

win32 {
    # use OpenCL headers from AMD APP Computing SDK
    INCLUDEPATH += \
    "$(AMDAPPSDKROOT)include" \

    LIBS += \
    -L"$(AMDAPPSDKROOT)lib/x86" \

}

unix:!macx {
    OPENCL_DIR = /usr/local/cuda
    INCLUDEPATH += $$OPENCL_DIR/include
}

macx {
    OPENCL_DIR = /usr/local/cuda
    INCLUDEPATH += $$OPENCL_DIR/include
}
}

# #######################################################################
# CUDA
# #######################################################################
usecufft: CONFIG += $${qtfeatures}cuda