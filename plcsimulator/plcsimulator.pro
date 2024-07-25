TARGET = plcsimulatordemo


QT += quick core widgets
CONFIG += qtquickcompiler
QMAKE_CXXFLAGS += -O3
CONFIG += c++1z

#DEFINES += ROS_SOFTWARE_VERSION=$$system(git describe)
linux:QMAKE_LFLAGS += -rdynamic

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += \
    datablocks

SOURCES +=\
    cliparser.cpp \
    main.cpp\
    plcmessage.cpp\
    treeitem.cpp\
    treemodel.cpp\
    udpclient.cpp\
    udpserver.cpp\
    testlogger.cpp \
    workerthread.cpp \
    dbparser.cpp\
    parser.cpp

HEADERS += \
    cliparser.h \
    plcmessage.h\
    treeitem.h\
    treemodel.h\
    udpclient.h\
    udpserver.h\
    testlogger.h \
    workerthread.h \
    signalhandlers.h \
    dbparser.h \
    parser.h

RESOURCES +=\
    qml.qrc\
    files.qrc


