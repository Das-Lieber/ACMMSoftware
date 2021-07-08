QT       += core gui opengl widgets

TARGET = ACMMSoftware
TEMPLATE = app

RC_FILE += logo.rc
RC_ICONS += main.ico

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    RLAPI_Reader.cpp \
    Robot.cpp \
    main.cpp \
    mainwindow.cpp \
    occ/GeneralTools.cpp \
    occ/LabelWidget.cpp \
    occ/MeasureItem.cpp \
    occ/RobotLink.cpp \
    occ/modelmaker.cpp \
    occ/occview.cpp \
    occ/pca.cpp \
    page/DMISReport.cpp \
    page/MeasureWidget.cpp \
    page/ReportViewer.cpp \
    page/constructionwidget.cpp \
    page/dialogPosition.cpp

HEADERS += \
    RLAPI_Reader.h \
    Robot.h \
    mainwindow.h \
    occ/GeneralTools.h \
    occ/LabelWidget.h \
    occ/MeasureItem.h \
    occ/RobotLink.h \
    occ/modelmaker.h \
    occ/occview.h \
    occ/pca.h \
    page/DMISReport.h \
    page/MeasureWidget.h \
    page/ReportViewer.h \
    page/constructionwidget.h \
    page/dialogPosition.h

FORMS += \
    page/MeasureWidget.ui \
    page/constructionwidget.ui \
    page/dialogPosition.ui

INCLUDEPATH += $$PWD/3rd/OpenCASCADE/inc
LIBS += $$PWD/3rd/OpenCASCADE/lib/*.lib

DESTDIR = $$PWD/bin

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += /utf-8

include($$PWD/SARibbon/SARibbonBar.pri)
include($$PWD/ACMM/ACMM.pri)
