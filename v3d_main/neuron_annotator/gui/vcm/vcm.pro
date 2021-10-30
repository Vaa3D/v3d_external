QT += core
CONFIG      += debug
mac {
    CONFIG += x86 x86_64
    CONFIG -= app_bundle
    QMAKE_CXXFLAGS+=-O3
    CXXFLAGS+=-O3
}
HEADERS += ../VolumeColorModel.h
SOURCES += ../VolumeColorModel.cpp \
          benchmark_vcm.cpp

