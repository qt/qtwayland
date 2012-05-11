#ifndef WAYLANDSURFACETEXTUREMATERIAL_H
#define WAYLANDSURFACETEXTUREMATERIAL_H

#include <QtQuick/QSGSimpleMaterial>
#include <QtQuick/QSGTexture>
#include <QtGui/QOpenGLFunctions>

class WaylandSurfaceTextureState {
public:
    WaylandSurfaceTextureState()
        : m_texture(0)
    {}
    void setTexture(QSGTexture *texture) { m_texture = texture; }
    QSGTexture *texture() const { return m_texture; }

private:
    QSGTexture *m_texture;
};

class WaylandSurfaceTextureMaterial : public QSGSimpleMaterialShader<WaylandSurfaceTextureState>
{
    QSG_DECLARE_SIMPLE_SHADER(WaylandSurfaceTextureMaterial, WaylandSurfaceTextureState)
    public:

        QList<QByteArray> attributes() const;

        void updateState(const WaylandSurfaceTextureState *newState, const WaylandSurfaceTextureState *oldState);
protected:
        const char *vertexShader() const;
        const char *fragmentShader() const;
};

class WaylandSurfaceTextureOpaqueMaterial : public QSGSimpleMaterialShader<WaylandSurfaceTextureState>
{
    QSG_DECLARE_SIMPLE_SHADER(WaylandSurfaceTextureOpaqueMaterial, WaylandSurfaceTextureState)
    public:

        QList<QByteArray> attributes() const;

        void updateState(const WaylandSurfaceTextureState *newState, const WaylandSurfaceTextureState *oldState);
protected:
        const char *vertexShader() const;
        const char *fragmentShader() const;
};

#endif // WAYLANDSURFACETEXTUREMATERIAL_H
