####################
# base on common lib
message(doing it)
DEFINES += end2end

TEMPLATE = app
win32:TEMPLATE = vcapp

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
