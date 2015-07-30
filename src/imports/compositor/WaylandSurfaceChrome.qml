/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

Item {
    id: chrome

    width: childrenRect.width
    height: childrenRect.height

    property QtObject surface
    property Item view

    property bool automaticUseShellView: false
    property bool automaticDestroyOnSurfaceDestroy: true

    property bool followRequestedPosition: false
    property real requestedXPosition
    property real requestedYPosition

    signal surfaceDestroyed

    Connections {
        target: surface

        onShellViewCreated: {
            if (chrome.automaticUseShellView)
                view = surface.shellView;
        }

        onSurfaceDestroyed: {
            if (automaticDestroyOnSurfaceDestroy)
                chrome.destroy();
        }
    }

    Component.onCompleted: {
        if (chrome.automaticUseShellView && !view && surface.shellView)
            view = surface.shellView;
    }

    onViewChanged: {
        if (view) {
            view.parent = chrome;
            view.followRequestedPosition = Qt.binding(function() { return chrome.followRequestedPosition; });
            chrome.visible = Qt.binding(function() { return view.visible; });
            chrome.requestedXPosition = Qt.binding(function() { return view.requestedXPosition; });
            chrome.requestedYPosition = Qt.binding(function() { return view.requestedYPosition; });
            view.surfaceDestroyed.connect(function() { chrome.surfaceDestroyed(); });
        } else {
            chrome.visible = false;
        }
    }

    Component.onDestruction: { if (view) view.destroy(); }
}
