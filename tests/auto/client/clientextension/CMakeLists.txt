qt_internal_add_test(tst_clientextension
    SOURCES
        tst_clientextension.cpp
    PUBLIC_LIBRARIES
        SharedClientTest
)

qt6_generate_wayland_protocol_client_sources(tst_clientextension
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.xml
)

qt6_generate_wayland_protocol_server_sources(tst_clientextension
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.xml
)
