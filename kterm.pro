SOURCES += main.cpp tabwindow.cpp
HEADERS += main.h tabwindow.h
TARGET = kterm

CONFIG += qt
QT += widgets
QT += KParts
QT += dbus
QT += x11extras

DBUS_ADAPTORS += org.kterm.xml
DBUS_INTERFACES += org.kterm.xml
