#include "surfacerenderer.h"

#include <QOpenGLFunctions>

SurfaceRenderer::SurfaceRenderer(QOpenGLContext *context, QWindow *surface)
    : m_context(context)
    , m_surface(surface)
{
    const char *textureVertexProgram =
            "uniform highp mat4 matrix;\n"
            "attribute highp vec3 vertexCoordEntry;\n"
            "attribute highp vec2 textureCoordEntry;\n"
            "varying highp vec2 textureCoord;\n"
            "void main() {\n"
            "   textureCoord = textureCoordEntry;\n"
            "   gl_Position = matrix * vec4(vertexCoordEntry, 1);\n"
            "}\n";

    const char *textureFragmentProgram =
            "uniform sampler2D texture;\n"
            "varying highp vec2 textureCoord;\n"
            "void main() {\n"
            "   gl_FragColor = texture2D(texture, textureCoord);\n"
            "}\n";

    m_context->makeCurrent(m_surface);

    //Enable transparent windows
    glEnable(GL_BLEND);
    glBlendFunc (GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

    //May need to manually set context here
    m_shaderProgram = new QGLShaderProgram();

    m_shaderProgram->addShaderFromSourceCode(QGLShader::Vertex, textureVertexProgram);
    m_shaderProgram->addShaderFromSourceCode(QGLShader::Fragment, textureFragmentProgram);
    m_shaderProgram->link();
    m_shaderProgram->bind();

    m_vertexCoordEntry = m_shaderProgram->attributeLocation("vertexCoordEntry");
    m_textureCoordEntry = m_shaderProgram->attributeLocation("textureCoordEntry");
    m_matrixLocation = m_shaderProgram->uniformLocation("matrix");

}

void SurfaceRenderer::drawImage(const QImage &image, const QRectF &geometry)
{
    if (image.isNull())
        return;
    drawTexture(textureFromImage(image), geometry);
}

void SurfaceRenderer::drawTexture(int textureId, const QRectF &geometry, int depth)
{
    GLfloat zValue = depth / 1000.0f;
    //Set Texture and Vertex coordinates
    GLfloat textureCoordinates[] = { 0, 1,
                                     1, 1,
                                     1, 0,
                                     0, 0
                                   };

    GLfloat vertexCoordinates[] = { geometry.left(), geometry.top(), zValue,
                                    geometry.right(), geometry.top(), zValue,
                                    geometry.right(), geometry.bottom(), zValue,
                                    geometry.left(), geometry.bottom(), zValue
                                  };

    //Set matrix to transfrom geometry values into gl coordinate space.
    m_transformMatrix.setToIdentity();
    m_transformMatrix.scale( 2.0f / m_surface->geometry().width(), -2.0f / m_surface->geometry().height());
    m_transformMatrix.translate(-m_surface->geometry().width() / 2.0f, -m_surface->geometry().height() / 2.0f);

    m_shaderProgram->bind();

    //attach the data!
    m_context->functions()->glEnableVertexAttribArray(m_vertexCoordEntry);
    m_context->functions()->glEnableVertexAttribArray(m_textureCoordEntry);

    m_context->functions()->glVertexAttribPointer(m_vertexCoordEntry, 3, GL_FLOAT, GL_FALSE, 0, vertexCoordinates);
    m_context->functions()->glVertexAttribPointer(m_textureCoordEntry, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinates);
    m_shaderProgram->setUniformValue(m_matrixLocation, m_transformMatrix);

    glBindTexture(GL_TEXTURE_2D, textureId);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_context->functions()->glDisableVertexAttribArray(m_vertexCoordEntry);
    m_context->functions()->glDisableVertexAttribArray(m_textureCoordEntry);
    m_shaderProgram->release();
}

GLuint SurfaceRenderer::textureFromImage(const QImage &image)
{
    //TODO: Replace this line
    QImage convertedImage = QGLWidget::convertToGLFormat(image);

    GLuint textureId;
    //Copy QImage data to Texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.width(), convertedImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedImage.constBits());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return textureId;
}
