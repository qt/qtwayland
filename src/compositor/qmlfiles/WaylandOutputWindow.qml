// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Window {
    id: window
    property QtObject compositor
    property QtObject output
    property bool automaticFrameCallback: false

    Component.onCompleted: {
        if (!compositor) {
            console.warn("WaylandOutputWindow initiated without compositor. This leads to undefined behavior");
            return;
        }
        output = compositor.addOutput(window);
        output.automaticFrameCallbacks = window.automaticFrameCallback;
    }
}

