#-------------------------------------------------
#
# Project created by QtCreator 2017-02-16T10:39:12
#
#-------------------------------------------------

QT       += core gui sql widgets network

TARGET = oprichnik_spo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        u_log.cpp \
    sv_pgdb.cpp

HEADERS  += mainwindow.h \
    u_log.h \
    pg_scripts.h \
    sv_pgdb.h

FORMS    += mainwindow.ui
