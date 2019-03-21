include (../shared/shared.pri)

qtConfig(cursor) {
    QMAKE_USE += wayland-cursor
    QT += gui-private
}

TARGET = tst_seatv4
SOURCES += tst_seatv4.cpp
