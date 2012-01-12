#include "qwindowcompositor.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTouchEvent>
#include <QOpenGLFunctions>

#include <QtCompositor/waylandinput.h>

QWindowCompositor::QWindowCompositor(QOpenGLWindow *window)
    : WaylandCompositor(window)
    , m_window(window)
    , m_textureBlitter(0)
    , m_renderScheduler(this)
    , m_draggingWindow(0)
    , m_dragKeyIsPressed(false)
{
    enableSubSurfaceExtension();
    m_window->makeCurrent();

    initializeGLFunctions();

    m_textureCache = new QOpenGLTextureCache(m_window->context());
    m_textureBlitter = new TextureBlitter();
    m_backgroundImage = QImage(QLatin1String(":/background.jpg"));
    m_renderScheduler.setSingleShot(true);
    connect(&m_renderScheduler,SIGNAL(timeout()),this,SLOT(render()));

    QOpenGLFunctions *functions = m_window->context()->functions();
    functions->glGenFramebuffers(1, &m_surface_fbo);

    window->installEventFilter(this);

    setRetainedSelectionEnabled(true);

    m_renderScheduler.start(0);
}

void QWindowCompositor::surfaceDestroyed(QObject *object)
{
    WaylandSurface *surface = static_cast<WaylandSurface *>(object);
    m_surfaces.removeOne(surface);
    if (defaultInputDevice()->keyboardFocus() == surface || !defaultInputDevice()->keyboardFocus()) // typically reset to 0 already in Compositor::surfaceDestroyed()
        defaultInputDevice()->setKeyboardFocus(m_surfaces.isEmpty() ? 0 : m_surfaces.last());
    m_renderScheduler.start(0);
}

void QWindowCompositor::surfaceMapped()
{
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    QPoint pos;
    if (!m_surfaces.contains(surface)) {
        uint px = 0;
        uint py = 0;
        if (!QCoreApplication::arguments().contains(QLatin1String("-stickytopleft"))) {
            px = 1 + (qrand() % (m_window->width() - surface->size().width() - 2));
            py = 1 + (qrand() % (m_window->height() - surface->size().height() - 2));
        }
        pos = QPoint(px, py);
        surface->setPos(pos);
    } else {
        surface->setPos(window()->geometry().topLeft());
        m_surfaces.removeOne(surface);
    }
    m_surfaces.append(surface);
    defaultInputDevice()->setKeyboardFocus(surface);
    m_renderScheduler.start(0);
}

void QWindowCompositor::surfaceDamaged(const QRect &rect)
{
    WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
    surfaceDamaged(surface, rect);
}

void QWindowCompositor::surfaceDamaged(WaylandSurface *surface, const QRect &rect)
{
    Q_UNUSED(surface)
    Q_UNUSED(rect)
    m_renderScheduler.start(0);
}

void QWindowCompositor::surfaceCreated(WaylandSurface *surface)
{
    connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
    connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
    m_renderScheduler.start(0);
}

QPointF QWindowCompositor::toSurface(WaylandSurface *surface, const QPointF &pos) const
{
    return pos - surface->pos();
}

void QWindowCompositor::changeCursor(const QImage &image, int hotspotX, int hotspotY)
{
    QCursor cursor(QPixmap::fromImage(image),hotspotX,hotspotY);
    static bool cursroIsSet = false;
    if (cursroIsSet) {
        QGuiApplication::changeOverrideCursor(cursor);
    } else {
        QGuiApplication::setOverrideCursor(cursor);
        cursroIsSet = true;
    }
}

WaylandSurface *QWindowCompositor::surfaceAt(const QPoint &point, QPoint *local)
{
    for (int i = m_surfaces.size() - 1; i >= 0; --i) {
        WaylandSurface *surface = m_surfaces.at(i);
        QRect geo(surface->pos().toPoint(),surface->size());
        if (geo.contains(point)) {
            if (local)
                *local = toSurface(surface, point).toPoint();
            return surface;
        }
    }
    return 0;
}

GLuint QWindowCompositor::composeSurface(WaylandSurface *surface)
{
    GLuint texture = 0;

    QOpenGLFunctions *functions = QOpenGLContext::currentContext()->functions();
    functions->glBindFramebuffer(GL_FRAMEBUFFER, m_surface_fbo);

    if (surface->type() == WaylandSurface::Shm) {
        texture = m_textureCache->bindTexture(QOpenGLContext::currentContext(),surface->image());
    } else {
        texture = surface->texture(QOpenGLContext::currentContext());
    }

    functions->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D, texture, 0);
    paintChildren(surface,surface);
    functions->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D,0, 0);

    functions->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return texture;
}

void QWindowCompositor::paintChildren(WaylandSurface *surface, WaylandSurface *window) {

    if (surface->subSurfaces().size() == 0)
        return;

    QLinkedListIterator<WaylandSurface *> i(surface->subSurfaces());
    while (i.hasNext()) {
        WaylandSurface *subSurface = i.next();
        QPointF p = subSurface->mapTo(window,QPointF(0,0));
        if (subSurface->size().isValid()) {
            GLuint texture = 0;
            if (subSurface->type() == WaylandSurface::Texture) {
                texture = subSurface->texture(QOpenGLContext::currentContext());
            } else if (surface->type() == WaylandSurface::Shm ) {
                texture = m_textureCache->bindTexture(QOpenGLContext::currentContext(),surface->image());
            }
            QRect geo(p.toPoint(),subSurface->size());
            m_textureBlitter->drawTexture(texture,geo,window->size(),0,window->isYInverted(),subSurface->isYInverted());
        }
        paintChildren(subSurface,window);
    }
}


void QWindowCompositor::render()
{
    m_window->makeCurrent();
    m_backgroundTexture = m_textureCache->bindTexture(QOpenGLContext::currentContext(),m_backgroundImage);

    m_textureBlitter->bind();
    //Draw the background Image texture
    int w = m_window->width();
    int h = m_window->height();
    QSize imageSize = m_backgroundImage.size();
    for (int y = 0; y < h; y += imageSize.height()) {
        for (int x = 0; x < w; x += imageSize.width()) {
            m_textureBlitter->drawTexture(m_backgroundTexture,QRect(QPoint(x, y),imageSize),window()->size(), 0,true,true);
        }
    }

    foreach (WaylandSurface *surface, m_surfaces) {
        GLuint texture = composeSurface(surface);
        QRect geo(surface->pos().toPoint(),surface->size());
        m_textureBlitter->drawTexture(texture,geo,m_window->size(),0,false,surface->isYInverted());
    }

    m_textureBlitter->release();
    frameFinished();
    glFinish();

    m_window->swapBuffers();
}

bool QWindowCompositor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_window)
        return false;

    WaylandInputDevice *input = defaultInputDevice();

    switch (event->type()) {
    case QEvent::Expose:
        m_renderScheduler.start(0);
        break;
    case QEvent::MouseButtonPress: {
        QPoint local;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        WaylandSurface *targetSurface = surfaceAt(me->pos(), &local);
        if (targetSurface) {
            if (m_dragKeyIsPressed) {
               m_draggingWindow = targetSurface;
               m_drag_diff = local;
            } else {
                if (input->keyboardFocus() != targetSurface) {
                    input->setKeyboardFocus(targetSurface);
                    m_surfaces.removeOne(targetSurface);
                    m_surfaces.append(targetSurface);
                    m_renderScheduler.start(0);
                }
                input->sendMousePressEvent(me->button(),local,me->pos());
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        WaylandSurface *targetSurface = input->mouseFocus();
        if (m_draggingWindow) {
            m_draggingWindow = 0;
            m_drag_diff = QPoint();
        } else  if (targetSurface) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            input->sendMouseReleaseEvent(me->button(),toSurface(targetSurface, me->pos()).toPoint(),me->pos());
        }
        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (m_draggingWindow) {
            m_draggingWindow->setPos(me->posF() - m_drag_diff);
            m_renderScheduler.start(0);
        } else {
            QPoint local;
            WaylandSurface *targetSurface = surfaceAt(me->pos(), &local);
            input->sendMouseMoveEvent(targetSurface, local,me->pos());
        }
        break;
    }
    case QEvent::KeyPress: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Meta || ke->key() == Qt::Key_Super_L) {
            m_dragKeyIsPressed = true;
        }
        WaylandSurface *targetSurface = input->keyboardFocus();
        if (targetSurface)
            input->sendKeyPressEvent(ke->nativeScanCode());
        break;
    }
    case QEvent::KeyRelease: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Meta || ke->key() == Qt::Key_Super_L) {
            m_dragKeyIsPressed = false;
        }
        WaylandSurface *targetSurface = input->keyboardFocus();
        if (targetSurface)
            input->sendKeyReleaseEvent(ke->nativeScanCode());
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        WaylandSurface *targetSurface = 0;
        QTouchEvent *te = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> points = te->touchPoints();
        QPoint pointPos;
        if (!points.isEmpty()) {
            pointPos = points.at(0).pos().toPoint();
            targetSurface = surfaceAt(pointPos);
        }
        if (targetSurface && targetSurface != input->mouseFocus())
            input->setMouseFocus(targetSurface, pointPos, pointPos);
        if (input->mouseFocus())
            input->sendFullTouchEvent(te);
        break;
    }
    default:
        break;
    }
    return false;
}
