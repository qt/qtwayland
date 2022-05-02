/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

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

