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

ShaderEffectItem {
    property variant source: null;
    property color color: "#ffffff"
    property real blend;

    onSourceChanged: {
        if (source != null) {
            source.setPaintEnabled(false);
            vertexShader = source.isYInverted() ? vShaderInvertedY : vShader;
        }
    }

    property string vShader: "
    uniform highp mat4 qt_ModelViewProjectionMatrix;
    attribute highp vec4 qt_Vertex;
    attribute highp vec2 qt_MultiTexCoord0;
    varying highp vec2 qt_TexCoord0;
    void main() {
        qt_TexCoord0 = qt_MultiTexCoord0;
        gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
    }
    "
    property string vShaderInvertedY: "
    uniform highp mat4 qt_ModelViewProjectionMatrix;
    attribute highp vec4 qt_Vertex;
    attribute highp vec2 qt_MultiTexCoord0;
    varying highp vec2 qt_TexCoord0;
    void main() {
        qt_TexCoord0 = vec2(0, 1) + qt_MultiTexCoord0 * vec2(1, -1);
        gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
    }
    "

    fragmentShader: "
    uniform sampler2D source;
    uniform float qt_Opacity;
    uniform vec4 color;
    uniform float blend;
    varying highp vec2 qt_TexCoord0;
    void main() {
        vec4 sourceColor = texture2D(source, qt_TexCoord0);
        vec3 delta = sourceColor.rgb - vec3(0.5);
        vec3 lowerContrast = vec3(0.5) + 0.4 * delta;
        gl_FragColor = qt_Opacity * mix(sourceColor, color * sourceColor.a * dot(lowerContrast, vec3(11, 16, 5) * (1. /  32.)), blend);
    }
    "
}
