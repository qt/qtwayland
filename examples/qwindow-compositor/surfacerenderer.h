#ifndef SURFACERENDERER_H
#define SURFACERENDERER_H

#include <QOpenGLContext>
#include <QGLShaderProgram>
#include <QWindow>

class SurfaceRenderer
{
public:
    SurfaceRenderer(QOpenGLContext *context, QWindow *surface);

    void drawImage(const QImage &image, const QRect &geometry);
    void drawTexture(int textureId, const QRect &geometry, int depth = 0);
    GLuint textureFromImage(const QImage &image);

private:

    QOpenGLContext *m_context;
    QWindow *m_surface;
    QGLShaderProgram *m_shaderProgram;
    QMatrix4x4 m_transformMatrix;

    int m_matrixLocation;
    int m_vertexCoordEntry;
    int m_textureCoordEntry;
};

#endif // SURFACERENDERER_H
