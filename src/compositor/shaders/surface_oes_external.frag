// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// This shader stump cannot be precompiled and is compiled at run-time.
// Appropriate target preamble added when it is loaded.

varying vec2 v_texcoord;
struct buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};
uniform buf ubuf;
uniform samplerExternalOES tex0;

void main()
{
    gl_FragColor = ubuf.qt_Opacity * texture2D(tex0, v_texcoord);
}
