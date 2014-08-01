include(../../common.pri)

TEMPLATE = app
TARGET = tst_genericimage
QT += network sql testlib

DEFINES += NO_KEY_PROVIDER

INCLUDEPATH += ../../src/lib/
INCLUDEPATH += ../../src/qml/

HEADERS +=  ../../src/lib/semaphore_p.h \
            ../../src/lib/socialsyncinterface.h \
            ../../src/lib/abstractsocialcachedatabase.h \
            ../../src/lib/abstractsocialcachedatabase_p.h \
            ../../src/lib/genericimagesdatabase.h \
            ../../src/lib/abstractimagedownloader.h \
            ../../src/lib/abstractimagedownloader_p.h \
            ../../src/qml/abstractsocialcachemodel.h \
            ../../src/qml/abstractsocialcachemodel_p.h \
            ../../src/qml/generic/genericimagecachemodel.h \
            ../../src/qml/generic/genericimagedownloader_p.h \
            ../../src/qml/generic/genericimagedownloader.h

SOURCES +=  ../../src/lib/semaphore_p.cpp \
            ../../src/lib/socialsyncinterface.cpp \
            ../../src/lib/abstractsocialcachedatabase.cpp \
            ../../src/lib/genericimagesdatabase.cpp \
            ../../src/lib/abstractimagedownloader.cpp \
            ../../src/qml/abstractsocialcachemodel.cpp \
            ../../src/qml/generic/genericimagecachemodel.cpp \
            ../../src/qml/generic/genericimagedownloader.cpp \
            main.cpp

target.path = /opt/tests/libsocialcache
INSTALLS += target
