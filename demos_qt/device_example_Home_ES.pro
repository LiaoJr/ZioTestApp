#运行时打印日志,注释DEFINES即关闭对应等级的日志
DEFINES += ENABLE_IOT_INFO=true
DEFINES += ENABLE_IOT_WARN=true
DEFINES += ENABLE_IOT_ERROR=true

# zws sdk
include("../zws_sdk.pri")

#定义文件生成名称
TARGET = device_example_Home_ES

#自定义生成位置
DESTDIR = ../bin/
#MOC_DIR = ../bin/qt
#RCC_DIR = ../bin/qt
#UI_DIR = ../bin/qt
#OBJECTS_DIR = ../bin/qt

QT += core
QT -= gui

CONFIG += c++11

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    ../demos/src/Home_ES/commands.h \
    ../demos/src/device_consts.h \
    ../demos/src/log.h \
    ../demos/src/Home_ES/device_Home_ES.h \
    ../demos/src/errors.h \
    ../demos/src/warnings.h
    
SOURCES += \
    ../demos/src/Home_ES/device_expl_Home_ES.c \
    ../demos/src/commands.c \
    ../demos/src/Home_ES/device_Home_ES.c \
    ../demos/src/errors.c \
    ../demos/src/settings.c \
    ../demos/src/warnings.c
)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

