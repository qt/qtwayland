# Generated from wlscaler.pro.

#####################################################################
## wlscaler Binary:
#####################################################################

qt_add_manual_test(wlscaler
    GUI
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Qml
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_add_resource(wlscaler "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:wlscaler.pro:<TRUE>:
# TEMPLATE = "app"
