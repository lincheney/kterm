SOURCES += main.cpp tabwindow.cpp x.cpp
HEADERS += main.h tabwindow.h x.h
TARGET = kterm

CONFIG += qt
QT += widgets
QT += KParts
QT += dbus
QT += x11extras

DBUS_ADAPTORS += org.kterm.xml
DBUS_INTERFACES += org.kterm.xml

DEFINES += 'GIT_REF="\\"\$$(git rev-parse HEAD)\\""'
