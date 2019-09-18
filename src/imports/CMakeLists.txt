# Generated from imports.pro.

if(TARGET Qt::Quick AND TARGET Qt::WaylandCompositor)
    add_subdirectory(compositor)
endif()
if(QT_FEATURE_opengl AND TARGET Qt::Quick AND TARGET Qt::WaylandClient AND TARGET Qt::WaylandCompositor)
    add_subdirectory(texture-sharing)
    add_subdirectory(texture-sharing-extension)
endif()
