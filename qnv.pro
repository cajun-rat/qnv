#-------------------------------------------------
#
# Project created by QtCreator 2013-11-13T08:32:20
#
#-------------------------------------------------

QT       += core gui

INCLUDEPATH += C:\src\boost_1_55_0
LIBPATH += C:\src\boost_1_55_0\stage\lib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qnv
TEMPLATE = app


SOURCES += main.cpp mainwindow.cpp notes.cpp

HEADERS  += mainwindow.h \
    notes.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc \
    resources.qrc


win32:RC_FILE = main.rc
