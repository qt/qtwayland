# Generated from qmlclient.pro.

#####################################################################
## qmlclient Binary:
#####################################################################

qt_internal_add_manual_test(qmlclient
    GUI
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Quick
        Qt::WaylandClient
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_internal_add_resource(qmlclient "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:qmlclient.pro:<TRUE>:
# TEMPLATE = "app"
