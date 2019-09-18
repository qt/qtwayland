# Generated from auto.pro.

if(TARGET Qt::WaylandClient)
    add_subdirectory(client)
endif()
if(TARGET Qt::WaylandClient AND TARGET Qt::WaylandCompositor)
    #add_subdirectory(cmake) # special case
endif()
if(TARGET Qt::WaylandCompositor)
    add_subdirectory(compositor)
endif()
