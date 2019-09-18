# Generated from qmlclient.pro.

#####################################################################
## qmlclient Binary:
#####################################################################

qt_add_manual_test(qmlclient
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::WaylandClient
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_add_resource(qmlclient "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:qmlclient.pro:<TRUE>:
# TEMPLATE = "app"
