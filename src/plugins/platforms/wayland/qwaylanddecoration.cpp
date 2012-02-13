#include "qwaylanddecoration.h"

#include "qwaylandwindow.h"
#include "qwaylandshmbackingstore.h"
#include "qwaylandshellsurface.h"
#include "qwaylandinputdevice.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QRadialGradient>
QWaylandDecoration::QWaylandDecoration(QWindow *window, QWaylandShmBackingStore *backing_store)
    : m_window(window)
    , m_wayland_window(static_cast<QWaylandWindow *>(window->handle()))
    , m_backing_store(backing_store)
    , m_margins(3,30,3,3)
    , m_mouseButtons(Qt::NoButton)
    , m_hasSetCursor(false)
{
    m_wayland_window->setDecoration(this);
    QTextOption option(Qt::AlignHCenter | Qt::AlignVCenter);
    option.setWrapMode(QTextOption::NoWrap);
    m_windowTitle.setTextOption(option);
}

QWaylandDecoration::~QWaylandDecoration()
{
    m_wayland_window->setDecoration(0);
}

void QWaylandDecoration::paintDecoration()
{
    QRect surfaceRect(QPoint(), m_backing_store->entireSurface()->size());
    QRect clips[] =
    {
        QRect(0, 0, surfaceRect.width(), m_margins.top()),
        QRect(0, surfaceRect.height() - m_margins.bottom(), surfaceRect.width(), m_margins.bottom()),
        QRect(0, m_margins.top(), m_margins.left(), surfaceRect.height() - m_margins.top() - m_margins.bottom()),
        QRect(surfaceRect.width() - m_margins.right(), m_margins.top(), m_margins.left(), surfaceRect.height() - m_margins.top() - m_margins.bottom())
    };
    QRect top = clips[0];
    QPainter p(m_backing_store->entireSurface());
    p.setRenderHint(QPainter::Antialiasing);
    QPoint gradCenter(top.center()+ QPoint(30,60));
    QRadialGradient grad(gradCenter, top.width() / 2, gradCenter);
    QColor base(90, 90, 100);
    grad.setColorAt(1, base);
    grad.setColorAt(0, base.lighter(123));
    QPainterPath roundedRect;
    roundedRect.addRoundedRect(surfaceRect, 6, 6);
    for (int i = 0; i < 4; ++i) {
        p.save();
        p.setClipRect(clips[i]);
        p.fillPath(roundedRect, grad);
        p.restore();
    }


    QString windowTitleText = m_window->windowTitle();
    if (!windowTitleText.isEmpty()) {
        if (m_windowTitle.text() != windowTitleText) {
            m_windowTitle.setText(windowTitleText);
            m_windowTitle.prepare();
        }
        p.save();
        p.setClipRect(top);
        p.setPen(QColor(0xee,0xee,0xee));
        QSizeF size = m_windowTitle.size();
        int dx = (top.width() - size.width()) /2;
        int dy = (top.height()- size.height()) /2;
        QPoint windowTitlePoint(top.topLeft().x() + dx,
                 top.topLeft().y() + dy);
        p.drawStaticText(windowTitlePoint,m_windowTitle);
        p.restore();
    }
}

void QWaylandDecoration::handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::MouseButtons b, Qt::KeyboardModifiers mods)

{
    //figure out what area mouse is in
    if (local.y() <= m_margins.top()) {
        processMouseTop(inputDevice,local,b,mods);
    } else if (local.y() > m_window->height() - m_margins.bottom()) {
        processMouseBottom(inputDevice,local,b,mods);
    } else if (local.x() <= m_margins.left()) {
        processMouseLeft(inputDevice,local,b,mods);
    } else if (local.x() > m_window->width() - m_margins.right()) {
        processMouseRight(inputDevice,local,b,mods);
    } else {
        restoreMouseCursor();
    }
    m_mouseButtons = b;
}

void QWaylandDecoration::restoreMouseCursor()
{
    if (m_hasSetCursor) {
        QGuiApplication::restoreOverrideCursor();
        m_hasSetCursor = false;
    }
}

bool QWaylandDecoration::inMouseButtonPressedState() const
{
    return m_mouseButtons & Qt::NoButton;
}

void QWaylandDecoration::startResize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize resize, Qt::MouseButtons buttons)
{
    if (isLeftClicked(buttons)) {
        m_wayland_window->shellSurface()->resize(inputDevice, resize);
        inputDevice->removeMouseButtonFromState(Qt::LeftButton);
    }
}

void QWaylandDecoration::startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons)
{
    if (isLeftClicked(buttons)) {
        m_wayland_window->shellSurface()->move(inputDevice);
        inputDevice->removeMouseButtonFromState(Qt::LeftButton);
    }
}

void QWaylandDecoration::processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    if (local.y() <= m_margins.bottom()) {
        if (local.x() <= margins().left()) {
            //top left bit
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_LEFT,b);
        } else if (local.x() > m_window->width() - margins().right()) {
            //top right bit
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_RIGHT,b);
        } else {
            //top reszie bit
            overrideCursor(Qt::SplitVCursor);
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP,b);
        }
    } else {
        restoreMouseCursor();
        startMove(inputDevice,b);
    }

}

void QWaylandDecoration::processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    if (local.x() <= margins().left()) {
        //bottom left bit
        startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT,b);
    } else if (local.x() > m_window->width() - margins().right()) {
        //bottom right bit
            startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT,b);
    } else {
        //bottom bit
        overrideCursor(Qt::SplitVCursor);
        startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_BOTTOM,b);
    }
}

void QWaylandDecoration::processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
    overrideCursor(Qt::SplitHCursor);
    startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_LEFT,b);
}

void QWaylandDecoration::processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
    overrideCursor(Qt::SplitHCursor);
    startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_RIGHT,b);
}

bool QWaylandDecoration::isLeftClicked(Qt::MouseButtons newMouseButtonState)
{
    if ((!m_mouseButtons & Qt::LeftButton) && (newMouseButtonState & Qt::LeftButton))
        return true;
    return false;
}

bool QWaylandDecoration::isLeftReleased(Qt::MouseButtons newMouseButtonState)
{
    if ((m_mouseButtons & Qt::LeftButton) && !(newMouseButtonState & Qt::LeftButton))
        return true;
    return false;
}
