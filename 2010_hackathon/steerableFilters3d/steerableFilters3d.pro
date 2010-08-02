TEMPLATE = lib
CONFIG += plugin \
    warning_off
INCLUDEPATH += ../../basic_c_fun /usr/include/opencv
HEADERS = steerPlugin.h \
    steerdialog.h \
    SteerableFilter3D.h \
    polynomial.h \
    Cube.h \
    utils.h
SOURCES = steerPlugin.cpp \
    steerdialog.cpp \
    SteerableFilter3D.cpp \
    utils.cpp
LIBS += `pkg-config --libs opencv`
TARGET = $$qtLibraryTarget(steerableFilters3d)
DESTDIR = ../../../plugins/steerableFilters3d
FORMS += steerdialog.ui
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
