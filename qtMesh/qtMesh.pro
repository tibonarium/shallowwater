#-------------------------------------------------
#
# Project created by QtCreator 2015-02-25T12:11:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = qtMesh
TEMPLATE = app


SOURCES += \
    main.cpp \
    mqmesh.cpp \
    Triangle_Mesh_Physical_Geometry_Data.cpp

HEADERS  += \
    mqmesh.h \
    Geometry_Mesh.h \
    Physical_Geometry_Data.h

FORMS    += \
    mqmesh.ui
