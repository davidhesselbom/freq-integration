####################
# base on common lib
DEFINES += end2end

TEMPLATE = app
win32:TEMPLATE = vcapp

macx: QMAKE_INFO_PLIST = disableretina.plist

CONFIG += c++11 sawetestlib
CONFIG += console

QT += testlib


INCLUDEPATH += \
    ../0-common \


win32 {
    LIB_OUTPUT_FOLDER=release
    CONFIG(debug, debug|release):LIB_OUTPUT_FOLDER=debug

    LIBS += \
        -L../0-common/$$LIB_OUTPUT_FOLDER -lcommon \
		
}

unix {
	LIBS += \
        -L../0-common -lcommon
}
