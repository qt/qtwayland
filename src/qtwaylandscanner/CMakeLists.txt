# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qtwaylandscanner.pro.

#####################################################################
## qtwaylandscanner Tool:
#####################################################################

qt_get_tool_target_name(target_name qtwaylandscanner)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET WaylandScanner # special case
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        qtwaylandscanner.cpp
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:qtwaylandscanner.pro:<TRUE>:
# _OPTION = "host_build"

# special case begin
# Abuse the function to make sure the package providing qtwaylandscanner calls
# find_package(WaylandScanner), aka the non-qt provided package.
qt_record_extra_package_dependency(qtwaylandscanner WaylandScanner "")
# special case end
