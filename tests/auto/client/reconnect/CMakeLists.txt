#####################################################################
## tst_client Test:
#####################################################################

qt_internal_add_test(tst_reconnect
    SOURCES
        wl-socket.c
        tst_reconnect.cpp
    PUBLIC_LIBRARIES
        SharedClientTest
        )
