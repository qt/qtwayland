#####################################################################
## tst_wl_reconnect Test:
#####################################################################

qt_internal_add_test(tst_wl_reconnect
    SOURCES
        wl-socket.c
        tst_reconnect.cpp
    LIBRARIES
        SharedClientTest
)
