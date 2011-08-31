#ifndef QOPENGLWINDOW_H
#define QOPENGLWINDOW_H

#include <QWindow>
#include <QOpenGLContext>
#include <QSurfaceFormat>

class QOpenGLWindow : public QWindow
{
public:
    QOpenGLWindow(const QSurfaceFormat &format, const QRect &geometry);
public:
    QOpenGLContext* context() { return m_context; }
    bool makeCurrent() { return m_context->makeCurrent(this); }
    void swapBuffers() { m_context->swapBuffers(this); }

private:
    QOpenGLContext *m_context;
    QSurfaceFormat m_format;
};

#endif // QOPENGLWINDOW_H
