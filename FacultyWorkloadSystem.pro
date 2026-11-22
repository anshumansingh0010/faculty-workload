QT += core gui widgets sql

CONFIG += c++17

TARGET = FacultyWorkloadSystem
TEMPLATE = app

SOURCES += \
    main.cpp \
    logindialog.cpp \
    mainwindow.cpp \
    database.cpp

HEADERS += \
    logindialog.h \
    mainwindow.h \
    database.h