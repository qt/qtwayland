# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_clientextension
    SOURCES
        tst_clientextension.cpp
    LIBRARIES
        SharedClientTest
)

qt6_generate_wayland_protocol_client_sources(tst_clientextension
    PRIVATE_CODE
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.xml
)

qt6_generate_wayland_protocol_server_sources(tst_clientextension
    PRIVATE_CODE
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.xml
)
