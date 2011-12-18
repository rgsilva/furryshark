#-------------------------------------------------
#
# Project created by QtCreator 2011-12-18T00:20:08
#
#-------------------------------------------------

QT       += core gui network

TARGET = Grooveshark
TEMPLATE = app



SOURCES += main.cpp\
        mainwindow.cpp \
    grooveshark.cpp \
    request.cpp \
    global.cpp \
    util.cpp

HEADERS  += mainwindow.h \
    grooveshark.h \
    request.h \
    global.h \
    util.h

LIBS += -lqjson

FORMS    += mainwindow.ui
