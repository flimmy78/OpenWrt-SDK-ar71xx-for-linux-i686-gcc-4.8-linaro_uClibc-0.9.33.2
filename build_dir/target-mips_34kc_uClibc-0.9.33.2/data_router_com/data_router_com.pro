TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    qcommunicatebase.c \
    qcommunicatemanage.c\
    common.c \
    listmanager.c \
    ramrt.c \
    httpcomm.c \
    apptcp.c


HEADERS += \
    qcommunicatebase.h \
    qcommunicatemanage.h \
    common.h \
    listmanager.h\
#    mosquitto.h \
    ramrt.h \
    httpcomm.h \
    apptcp.h

LIBS +=  -lpthread


