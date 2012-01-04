#ifndef TEXTUREBLITTER_H
#define TEXTUREBLITTER_H

#include <QtGui/QMatrix4x4>

#include <QtGui/qopengl.h>

class QOpenGLShaderProgram;
class TextureBlitter
{
public:
    TextureBlitter();
    void bind();
    void release();
    void drawTexture(int textureId, const QRectF &sourceGeometry,
                     const QSize &targetRect, int depth,
                     bool targethasInvertedY, bool sourceHasInvertedY);

private:
    QOpenGLShaderProgram *m_shaderProgram;
    QMatrix4x4 m_transformMatrix;

    int m_matrixLocation;
    int m_vertexCoordEntry;
    int m_textureCoordEntry;
};

#endif // TEXTUREBLITTER_H
