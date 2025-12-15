QT += core gui widgets sql

CONFIG += c++17

TARGET = CourseProject
TEMPLATE = app

DESTDIR = bin

SOURCES += \
    src/main.cpp \
    src/db/DatabaseManager.cpp \
    src/core/CryptoUtils.cpp \
    src/core/CourseManager.cpp \
    src/ui/LoginDialog.cpp \
    src/ui/AdminWindow.cpp

HEADERS += \
    src/db/DatabaseManager.h \
    src/models/Structures.h \
    src/core/CryptoUtils.h \
    src/core/CourseManager.h \
    src/ui/LoginDialog.h \
    src/ui/AdminWindow.h

# Include paths
INCLUDEPATH += src

# PostgreSQL support
unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libpq
}

# Debug configuration
CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}