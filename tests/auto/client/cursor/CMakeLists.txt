#####################################################################
## tst_cursor Test:
#####################################################################

qt_internal_add_test(tst_wayland_cursor
    SOURCES
        tst_cursor.cpp
        cursorshapev1.cpp
    LIBRARIES
        SharedClientTest
)
