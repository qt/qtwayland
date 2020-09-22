# Generated from scaling-compositor.pro.

#####################################################################
## scaling-compositor Binary:
#####################################################################

qt_internal_add_manual_test(scaling-compositor
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

qt_internal_add_resource(scaling-compositor "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:scaling-compositor.pro:<TRUE>:
# TEMPLATE = "app"
