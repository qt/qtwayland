#version 440
#extension GL_OES_EGL_image_external_essl3 : require

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

layout(binding = 1) uniform samplerExternalOES tex0;

void main()
{
    fragColor = qt_Opacity * texture(tex0, v_texcoord);
}
