/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

import QtQuick 2.0

Item {
    id: container

    x: -400;
    y: 0;
    opacity: 0

    property variant chrome: null;
    property variant child: null;
    property bool animationsEnabled: false;
    property int index;

    Behavior on x {
        enabled: container.animationsEnabled;
        NumberAnimation { easing.type: Easing.InCubic; duration: 200; }
    }

    Behavior on y {
        enabled: container.animationsEnabled;
        NumberAnimation { easing.type: Easing.InQuad; duration: 200; }
    }

    Behavior on scale {
        enabled: container.animationsEnabled;
        NumberAnimation { easing.type: Easing.InQuad; duration: 200; }
    }

    Behavior on opacity {
        enabled: true;
        NumberAnimation { easing.type: Easing.Linear; duration: 250; }
    }

    ShaderEffect {
        id: effect
        source: child
        anchors.fill: child
        blend: { if (child && child.focus) 0.0; else 0.6 }
        opacity: 0.8
        z: 1

        Behavior on blend {
            enabled: true;
            NumberAnimation { easing.type: Easing.Linear; duration: 200; }
        }
    }
}
