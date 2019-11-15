# Generated from qtwaylandscanner.pro.

#####################################################################
## qtwaylandscanner Tool:
#####################################################################

qt_add_tool(qtwaylandscanner
    TOOLS_TARGET WaylandScanner # special case
    SOURCES
        qtwaylandscanner.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:qtwaylandscanner.pro:<TRUE>:
# _OPTION = "host_build"

# special case begin
# Abuse the function to make sure the package providing qtwaylandscanner calls
# find_package(WaylandScanner), aka the non-qt provided package.
qt_record_extra_package_dependency(qtwaylandscanner WaylandScanner "")
# special case end
