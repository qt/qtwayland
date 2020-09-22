# Generated from wip-cpp-compositor.pro.

#####################################################################
## wip-cpp-compositor Binary:
#####################################################################

qt_internal_add_manual_test(wip-cpp-compositor
    GUI
    SOURCES
        compositor.cpp compositor.h
        main.cpp
        window.cpp window.h
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:wip-cpp-compositor.pro:<TRUE>:
# INSTALLS = "target"
# target.path = "$$[QT_INSTALL_EXAMPLES]/wayland/reference-cpp"
