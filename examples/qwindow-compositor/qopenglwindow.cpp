#include "qopenglwindow.h"

QOpenGLWindow::QOpenGLWindow(const QSurfaceFormat &format, const QRect &geometry)
    : m_format(format)
{
    setSurfaceType(QWindow::OpenGLSurface);
    setGeometry(geometry);
    setFormat(format);
    create();
    m_context = new QOpenGLContext;
    m_context->setFormat(format);
    m_context->create();
}
