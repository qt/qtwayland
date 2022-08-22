# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from client.pro.

add_subdirectory(shared)

add_subdirectory(client)
add_subdirectory(clientextension)
add_subdirectory(datadevicev1)
add_subdirectory(fullscreenshellv1)
add_subdirectory(iviapplication)
add_subdirectory(multithreaded)
add_subdirectory(nooutput)
add_subdirectory(output)
add_subdirectory(primaryselectionv1)
add_subdirectory(seatv4)
add_subdirectory(seat)
add_subdirectory(surface)
add_subdirectory(tabletv2)
add_subdirectory(wl_connect)
add_subdirectory(xdgdecorationv1)
add_subdirectory(xdgoutput)
add_subdirectory(xdgshell)
if(QT_FEATURE_im)
    add_subdirectory(inputcontext)
endif()
