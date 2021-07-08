QT += axcontainer network serialport

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/Decode/decode.h \
    $$PWD/Decode/fiftydecode.h \
    $$PWD/Decode/twentysixdecode.h \
    $$PWD/Method/calibration.h \
    $$PWD/Method/coordinate.h \
    $$PWD/Method/explanation.h \
    $$PWD/Method/method.h \
    $$PWD/Method/uniqueid.h \
    $$PWD/UI/calibrationform.h \
    $$PWD/UI/coordinateform.h \
    $$PWD/commondata.h \
    $$PWD/fileInOut/importcoefile.h \
    $$PWD/fileInOut/importfile.h \
    $$PWD/fileInOut/importstructfile.h \
    $$PWD/fileInOut/savefile.h \
    $$PWD/imagethread.h \
    $$PWD/serial.h

SOURCES += \
    $$PWD/Decode/decode.cpp \
    $$PWD/Decode/fiftydecode.cpp \
    $$PWD/Decode/twentysixdecode.cpp \
    $$PWD/Method/calibration.cpp \
    $$PWD/Method/coordinate.cpp \
    $$PWD/Method/explanation.cpp \
    $$PWD/Method/method.cpp \
    $$PWD/Method/uniqueid.cpp \
    $$PWD/UI/calibrationform.cpp \
    $$PWD/UI/coordinateform.cpp \
    $$PWD/commondata.cpp \
    $$PWD/fileInOut/importcoefile.cpp \
    $$PWD/fileInOut/importfile.cpp \
    $$PWD/fileInOut/importstructfile.cpp \
    $$PWD/fileInOut/savefile.cpp \
    $$PWD/imagethread.cpp \
    $$PWD/serial.cpp
