# Generated from seatv4.pro.

#####################################################################
## tst_seatv4 Test:
#####################################################################

qt_internal_add_test(tst_seatv4
    SOURCES
        tst_seatv4.cpp
    PUBLIC_LIBRARIES
        SharedClientTest
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_seatv4 CONDITION QT_FEATURE_cursor
    PUBLIC_LIBRARIES
        Qt::GuiPrivate
        Wayland::Cursor
)
