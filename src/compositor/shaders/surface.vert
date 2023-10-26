// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) in vec2 qt_VertexPosition;
layout(location = 1) in vec2 qt_VertexTexCoord;
layout(location = 0) out vec2 v_texcoord;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = qt_Matrix * vec4(qt_VertexPosition, 0.0, 1.0);
    v_texcoord = qt_VertexTexCoord;
}
