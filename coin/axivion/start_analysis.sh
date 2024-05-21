#!/bin/bash
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

$HOME/bauhaus-suite/setup.sh --non-interactive
export PATH=/home/qt/bauhaus-suite/bin:$PATH
export BAUHAUS_CONFIG=$(cd $(dirname $(readlink -f $0)) && pwd)
export AXIVION_VERSION_NAME=$(git rev-parse HEAD)
export EXCLUDE_FILES="build/*:src/3rdparty/*"
export CAFECC_BASEPATH="/home/qt/work/qt/$TESTED_MODULE_COIN"
gccsetup --cc gcc --cxx g++ --config "$BAUHAUS_CONFIG"
cd "$CAFECC_BASEPATH"
BAUHAUS_IR_COMPRESSION=none COMPILE_ONLY=1 cmake -G Ninja -DAXIVION_ANALYSIS_TOOLCHAIN_FILE=/home/qt/bauhaus-suite/profiles/cmake/axivion-launcher-toolchain.cmake -DCMAKE_PREFIX_PATH=/home/qt/work/qt/qtwayland/build -DCMAKE_PROJECT_INCLUDE_BEFORE=/home/qt/bauhaus-suite/profiles/cmake/axivion-before-project-hook.cmake -B build -S . --fresh
cmake --build build -j4
for MODULE in qtwaylandclient qtwaylandcompositor; do
    export MODULE
    export IRNAME=build/$MODULE.ir
    if [ "$MODULE" == "qtwaylandclient" ]
    then
        export TARGET_NAME="build/lib/libQt6WaylandClient.so.*.ir:build/lib/libQt6WaylandEglClientHwIntegration.so.*.ir:build/lib/libQt6WlShellIntegration.so.*.ir"
        export PLUGINS="build/plugins/*/*.so:build/qml/QtWayland/Client/*.so"
        export PACKAGE="Add-ons"
    elif [ "$MODULE" == "qtwaylandcompositor" ]
    then
        export TARGET_NAME="build/lib/libQt6WaylandCompositor.so.*.ir"
        export PLUGINS="build/qml/QtWayland/Compositor/*.so"
        export PACKAGE="Add-ons"
    fi
    axivion_ci "$@"
done
