// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

layout(binding = 1) uniform sampler2D tex0;
layout(binding = 2) uniform sampler2D tex1;
layout(binding = 3) uniform sampler2D tex2;

void main()
{
    float y = 1.16438356 * (texture(tex0, v_texcoord).x - 0.0625);
    float u = texture(tex1, v_texcoord).x - 0.5;
    float v = texture(tex2, v_texcoord).x - 0.5;
    y *= qt_Opacity;
    u *= qt_Opacity;
    v *= qt_Opacity;
    fragColor.r = y + 1.59602678 * v;
    fragColor.g = y - 0.39176229 * u - 0.81296764 * v;
    fragColor.b = y + 2.01723214 * u;
    fragColor.a = qt_Opacity;
}
