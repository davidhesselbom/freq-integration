#-------------------------------------------------
#
# Project created by QtCreator 2011-03-07T20:41:08
#
#-------------------------------------------------

QT += testlib
QT += opengl

TARGET = FFTmoj
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   -= gui

TEMPLATE = app
win32:TEMPLATE = vcapp

SONICAWE = ../../../../src

SOURCES += *.cpp \
    $$SONICAWE/tfr/fftooura.cpp \
    $$SONICAWE/tfr/fft4g.c \
    $$SONICAWE/tfr/fftimplementation.cpp \
    $$SONICAWE/tfr/stftkernel_cpu.cpp \
    $$SONICAWE/signal/*buffer.cpp \
    $$SONICAWE/tfr/complexbuffer.cpp \
    $$SONICAWE/signal/intervals.cpp \

HEADERS += \
    $$SONICAWE/tfr/fftooura.h \
    $$SONICAWE/tfr/fftimplementation.h \
    $$SONICAWE/tfr/stftkernel.h \
    $$SONICAWE/signal/*buffer.h \
    $$SONICAWE/tfr/complexbuffer.h \
    $$SONICAWE/signal/intervals.h \

# "Other files" for Qt Creator

DEFINES += SRCDIR=\\\"$$PWD/\\\"

DEFINES += SAWE_NODLL

unix:IS64 = $$system(if [ "`uname -m`" = "x86_64" ]; then echo 64; fi)

AUXLIB = ../../../../lib
WINLIB = $$AUXLIB/sonicawe-winlib
MACLIB = $$AUXLIB/sonicawe-maclib
GPUMISC = $$AUXLIB/gpumisc

INCLUDEPATH += $$GPUMISC $$SONICAWE

unix:!macx {
  LIBS += \
    -lglut \
    -L$$GPUMISC -lgpumisc \
    -lGLEW \

}

win32 {
INCLUDEPATH += \
    $$WINLIB \
    $$WINLIB/glew/include \
    $$WINLIB/glut \

LIBS += \
    -l$$WINLIB/glut/glut32 \
    -l$$WINLIB/glew/lib/glew32 \
    -L$$WINLIB/boostlib \

}

macx {
INCLUDEPATH += \
    $$MACLIB/boost_1_45_0
LIBS += \
    -framework GLUT \
    -L$$GPUMISC -lgpumisc
}


macx:QMAKE_LFLAGS += -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -m32 -arch i386
macx:QMAKE_CXXFLAGS += -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -m32 -arch i386 -Wfatal-errors
macx:QMAKE_CFLAGS += -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -m32 -arch i386 -Wfatal-errors

####################
# Temporary output

win32:RCC_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp/
UI_DIR = tmp

CONFIG(debug, debug|release):OBJECTS_DIR = tmp/debug/
else:OBJECTS_DIR = tmp/release/

# #######################################################################
# OpenCL
# #######################################################################
useopenclnvidia {
DEFINES += USE_OPENCL

SOURCES += \
    $$SONICAWE/tfr/clfft/*.cpp

HEADERS += \
    $$SONICAWE/tfr/clfft/*.h

macx: LIBS += -framework OpenCL
!macx: LIBS += -lOpenCL

win32 {
    # use OpenCL headers from AMD APP Computing SDK
    INCLUDEPATH += "$(AMDAPPSDKROOT)include"
    LIBS += -L"$(AMDAPPSDKROOT)lib/x86"

    # use OpenCL headers from Cuda Gpu Computing SDK
    #INCLUDEPATH += "$(CUDA_INC_PATH)"
    #LIBS += -L"$(CUDA_LIB_PATH)"
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

useopenclamd {
DEFINES += USE_OPENCL

SOURCES += \
    $$GPUMISC/openclcontext.cpp \
    $$GPUMISC/openclmemorystorage.cpp \
    $$SONICAWE/tfr/clamdfft/*.cpp

HEADERS += \
    $$GPUMISC/openclcontext.h \
    $$GPUMISC/openclmemorystorage.h \
    $$SONICAWE/tfr/clamdfft/*.h

macx: LIBS += -framework OpenCL
!macx: LIBS += -lOpenCL

win32 {
    # use OpenCL headers from AMD APP Computing SDK
    INCLUDEPATH += \
    "$(AMDAPPSDKROOT)include" \
    $$WINLIB/clamdfft/include \

    LIBS += \
    -L"$(AMDAPPSDKROOT)lib/x86" \
    -l$$WINLIB/clamdfft/lib32/import/clAmdFft.Runtime \

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

usecuda{
unix:!macx {
        QMAKE_CXX = g++-4.3
        QMAKE_CC = gcc-4.3
        QMAKE_LINK = g++-4.3
}



LIBS += -lcufft -lcudart -lcuda
CONFIG(debug, debug|release): CUDA_FLAGS += -g
CUDA_FLAGS += --use_fast_math
#CUDA_FLAGS += --ptxas-options=-v


CUDA_CXXFLAGS = $$QMAKE_CXXFLAGS
CONFIG(debug, debug|release):CUDA_CXXFLAGS += $$QMAKE_CXXFLAGS_DEBUG
else:CUDA_CXXFLAGS += $$QMAKE_CXXFLAGS_RELEASE
win32 {
    INCLUDEPATH += "$(CUDA_INC_PATH)"
    LIBS += -L"$(CUDA_LIB_PATH)"
    CUDA_CXXFLAGS -= -Zc:wchar_t-
    CUDA_CXXFLAGS += -Zc:wchar_t
    CUDA_CXXFLAGS += /EHsc
    cuda.output = $${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.obj
    cuda.commands = \"$(CUDA_BIN_PATH)/nvcc.exe\" \
                -ccbin $${QMAKE_CC} \
        -c \
        -Xcompiler \
        \"$$join(CUDA_CXXFLAGS," ")\" \
        $$join(INCLUDEPATH,'" -I "','-I "','"') \
        $$CUDA_FLAGS \
        "${QMAKE_FILE_NAME}" \
                -m32 -o \
        "${QMAKE_FILE_OUT}"
}
unix:!macx {
    # auto-detect CUDA path
    # CUDA_DIR = $$system(which nvcc | sed 's,/bin/nvcc$,,')
    CUDA_DIR = /usr/local/cuda
    INCLUDEPATH += $$CUDA_DIR/include
    QMAKE_LIBDIR += $$CUDA_DIR/lib$$IS64
    cuda.output = $${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
    cuda.commands = $${CUDA_DIR}/bin/nvcc \
                -ccbin $${QMAKE_CC} \
        -c \
        -Xcompiler \
        $$join(CUDA_CXXFLAGS,",") \
        $$join(INCLUDEPATH,'" -I "','-I "','"') \
        $$CUDA_FLAGS \
        ${QMAKE_FILE_NAME} \
        -o \
        ${QMAKE_FILE_OUT}
    cuda.dependcy_type = TYPE_C
    cuda.depend_command_dosntwork = nvcc \
        -M \
        -Xcompiler \
        $$join(CUDA_CXXFLAGS,",") \
        $$join(INCLUDEPATH,'" -I "','-I "','"') \
        ${QMAKE_FILE_NAME} \
        | \
        sed \
        "s,^.*: ,," \
        | \
        sed \
        "s,^ *,," \
        | \
        tr \
        -d \
        '\\\n'
}

# cuda.depends = nvcc -M -Xcompiler $$join(QMAKE_CXXFLAGS,",") $$join(INCLUDEPATH,'" -I "','-I "','"') ${QMAKE_FILE_NAME} | sed "s,^.*: ,," | sed "s,^ *,," | tr -d '\\\n'

macx {
    # auto-detect CUDA path
    # CUDA_DIR = $$system(which nvcc | sed 's,/bin/nvcc$,,')
    # manual
    CUDA_DIR = /usr/local/cuda
    INCLUDEPATH += $$CUDA_DIR/include
    QMAKE_LIBDIR += $$CUDA_DIR/lib
    cuda.output = $${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
    cuda.commands = $${CUDA_DIR}/bin/nvcc \
                -ccbin $${QMAKE_CC} \
        -c \
        -Xcompiler \
        $$join(CUDA_CXXFLAGS,",") \
        $$join(INCLUDEPATH,'" -I "','-I "','"') \
        $$CUDA_FLAGS \
        ${QMAKE_FILE_NAME} \
        -o \
        ${QMAKE_FILE_OUT}
    cuda.dependcy_type = TYPE_C
}

cuda.input = CUDA_SOURCES
QMAKE_EXTRA_COMPILERS += cuda

} # end of cuda section #######################################################################

