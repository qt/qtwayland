# Generated from hardwareintegration.pro.

if(TARGET Qt::WaylandClient)
    add_subdirectory(client)
endif()
if(TARGET Qt::WaylandCompositor)
    add_subdirectory(compositor)
endif()
