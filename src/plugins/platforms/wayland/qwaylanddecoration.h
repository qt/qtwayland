#ifndef QWAYLANDDECORATION_H
#define QWAYLANDDECORATION_H

#include <QtCore/QMargins>
#include <QtCore/QPointF>
#include <QtGui/QGuiApplication>
#include <QtGui/QCursor>
#include <QtGui/QImage>
#include <QtGui/QStaticText>

#include <wayland-client.h>

#include <QtCore/QDebug>

class QWindow;
class QPaintDevice;
class QPainter;
class QEvent;
class QWaylandWindow;
class QWaylandShmBackingStore;
class QWaylandInputDevice;

class QWaylandDecoration
{
public:
    QWaylandDecoration(QWindow *window, QWaylandShmBackingStore *backing_store);
    ~QWaylandDecoration();
    void paintDecoration();
    void handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void restoreMouseCursor();
    bool inMouseButtonPressedState() const;

    void startResize(QWaylandInputDevice *inputDevice,enum wl_shell_surface_resize resize, Qt::MouseButtons buttons);
    void startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons);
    QMargins margins() const;
private:
    void overrideCursor(Qt::CursorShape shape);

    void processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);

    bool isLeftClicked(Qt::MouseButtons newMouseButtonState);
    bool isLeftReleased(Qt::MouseButtons newMouseButtonState);

    QWindow *m_window;
    QWaylandWindow *m_wayland_window;
    QWaylandShmBackingStore *m_backing_store;

    QMargins m_margins;
    bool m_hasSetCursor;
    Qt::CursorShape m_cursorShape;
    Qt::MouseButtons m_mouseButtons;

    QStaticText m_windowTitle;

    QImage m_borderImage;
};

inline QMargins QWaylandDecoration::margins() const
{
    return m_margins;
}

inline void QWaylandDecoration::overrideCursor(Qt::CursorShape shape)
{
    if (m_hasSetCursor) {
        if (m_cursorShape != shape) {
            QGuiApplication::changeOverrideCursor(QCursor(shape));
            m_cursorShape = shape;
        }
    } else {
        QGuiApplication::setOverrideCursor(QCursor(shape));
        m_hasSetCursor = true;
        m_cursorShape = shape;
    }
}

#endif // QWAYLANDDECORATION_H
