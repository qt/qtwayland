#include "waylandsurfacetexturematerial.h"
#include <QtGui/QOpenGLContext>

static const char wayland_surface_texture_material_vertex[] =
           "uniform highp mat4 qt_Matrix;                      \n"
           "attribute highp vec4 qt_VertexPosition;            \n"
           "attribute highp vec2 qt_VertexTexCoord;            \n"
           "varying highp vec2 qt_TexCoord;                    \n"
           "void main() {                                      \n"
           "    qt_TexCoord = qt_VertexTexCoord;               \n"
           "    gl_Position = qt_Matrix * qt_VertexPosition;   \n"
           "}";


static const char wayland_surface_texture_opaque_material_fragment[] =
           "varying highp vec2 qt_TexCoord;                    \n"
           "uniform sampler2D qt_Texture;                      \n"
           "uniform lowp float qt_Opacity;                     \n"
           "void main() {                                      \n"
           "    gl_FragColor = vec4(texture2D(qt_Texture, qt_TexCoord).rgb, 1.0) * qt_Opacity; \n"
           "}";

static const char wayland_surface_texture_material_fragment[] =
           "varying highp vec2 qt_TexCoord;                    \n"
           "uniform sampler2D qt_Texture;                      \n"
           "uniform lowp float qt_Opacity;                     \n"
           "void main() {                                      \n"
           "    gl_FragColor = texture2D(qt_Texture, qt_TexCoord) * qt_Opacity; \n"
           "}";

QList<QByteArray> WaylandSurfaceTextureMaterial::attributes() const
{
    QList<QByteArray> attributeList;
    attributeList << "qt_VertexPosition";
    attributeList << "qt_VertexTexCoord";
    return attributeList;
}

void WaylandSurfaceTextureMaterial::updateState(const WaylandSurfaceTextureState *newState, const WaylandSurfaceTextureState *oldState)
{
    Q_UNUSED(oldState);
    newState->texture()->bind();
}

const char *WaylandSurfaceTextureMaterial::vertexShader() const
{
    return wayland_surface_texture_material_vertex;
}

const char *WaylandSurfaceTextureMaterial::fragmentShader() const
{
    return wayland_surface_texture_material_fragment;
}

QList<QByteArray> WaylandSurfaceTextureOpaqueMaterial::attributes() const
{
    QList<QByteArray> attributeList;
    attributeList << "qt_VertexPosition";
    attributeList << "qt_VertexTexCoord";
    return attributeList;
}

void WaylandSurfaceTextureOpaqueMaterial::updateState(const WaylandSurfaceTextureState *newState, const WaylandSurfaceTextureState *oldState)
{
    Q_UNUSED(oldState);
    newState->texture()->bind();
}

const char *WaylandSurfaceTextureOpaqueMaterial::vertexShader() const
{
    return wayland_surface_texture_material_vertex;
}

const char *WaylandSurfaceTextureOpaqueMaterial::fragmentShader() const
{
    return wayland_surface_texture_opaque_material_fragment;
}
