/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwlshellsurface_p.h"

#include "qwlcompositor_p.h"
#include "qwlsurface_p.h"
#include "qwloutput_p.h"
#include "qwlinputdevice_p.h"
#include "qwlsubsurface_p.h"
#include "qwlpointer_p.h"
#include "qwlextendedsurface_p.h"

#include "qwaylandview.h"
#include "qwaylandoutput.h"
#include "qwaylandview.h"

#include <QtCore/qglobal.h>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland {

Shell::Shell(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate(compositor)
    , wl_shell(compositor->waylandDisplay(), 1)
{
}

ShellSurfacePopupGrabber *Shell::getPopupGrabber(QWaylandInputDevice *input)
{
    if (!m_popupGrabber.contains(input))
        m_popupGrabber.insert(input, new ShellSurfacePopupGrabber(input));

    return m_popupGrabber.value(input);
}

void Shell::shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface_res)
{
    Surface *surface = Surface::fromResource(surface_res);
    new ShellSurface(this, resource->client(), id, surface);
}

ShellSurface::ShellSurface(Shell *shell, wl_client *client, uint32_t id, Surface *surface)
    : QWaylandExtensionTemplate(surface->waylandSurface())
    , wl_shell_surface(client, id, 1)
    , m_shell(shell)
    , m_surface(surface)
    , m_resizeGrabber(0)
    , m_moveGrabber(0)
    , m_popupGrabber(0)
    , m_popupSerial()
    , m_surfaceType(None)
    , m_transientInactive(false)
    , m_transientParent(0)
    , m_transientOffset()
{
    m_view = surface->compositor()->waylandCompositor()->createSurfaceView(surface->waylandSurface());
    m_view->setOutput(surface->waylandSurface()->primaryOutput());
    connect(surface->waylandSurface(), &QWaylandSurface::configure, this, &ShellSurface::configure);
    connect(surface->waylandSurface(), &QWaylandSurface::mapped, this, &ShellSurface::mapped);
    connect(surface->waylandSurface(), &QWaylandSurface::offsetForNextFrame, this, &ShellSurface::adjustOffset);
}

ShellSurface::~ShellSurface()
{
    delete m_view;
}

void ShellSurface::sendConfigure(uint32_t edges, int32_t width, int32_t height)
{
    send_configure(edges, width, height);
}

void ShellSurface::ping()
{
    uint32_t serial = wl_display_next_serial(m_surface->compositor()->wl_display());
    ping(serial);
}

void ShellSurface::ping(uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(serial);
}

void ShellSurface::setSurfaceType(SurfaceType type)
{
    if (m_surfaceType == type)
        return;

    m_surfaceType = type;
    emit surfaceTypeChanged();
}

ShellSurface::SurfaceType ShellSurface::surfaceType() const
{
    return m_surfaceType;
}

void ShellSurface::adjustPosInResize()
{
    if (transientParent())
        return;
    if (!m_resizeGrabber || !(m_resizeGrabber->resize_edges & WL_SHELL_SURFACE_RESIZE_TOP_LEFT))
        return;

    int bottomLeftX = m_resizeGrabber->point.x() + m_resizeGrabber->width;
    int bottomLeftY = m_resizeGrabber->point.y() + m_resizeGrabber->height;
    qreal x = m_view->requestedPosition().x();
    qreal y = m_view->requestedPosition().y();
    if (m_resizeGrabber->resize_edges & WL_SHELL_SURFACE_RESIZE_TOP)
        y = bottomLeftY - m_view->surface()->size().height();
    if (m_resizeGrabber->resize_edges & WL_SHELL_SURFACE_RESIZE_LEFT)
        x = bottomLeftX - m_view->surface()->size().width();
    QPointF newPos(x,y);
    m_view->setRequestedPosition(newPos);
}

void ShellSurface::resetResizeGrabber()
{
    m_resizeGrabber = 0;
}

void ShellSurface::resetMoveGrabber()
{
    m_moveGrabber = 0;
}

void ShellSurface::setOffset(const QPointF &offset)
{
    setTransientOffset(offset);
}

void ShellSurface::configure(bool hasBuffer)
{
    m_surface->setMapped(hasBuffer);
}

void ShellSurface::mapped()
{
    if (m_surfaceType == Popup) {
        if (m_surface->mapped() && m_popupGrabber->grabSerial() == m_popupSerial) {
            m_popupGrabber->addPopup(this);
        } else {
            send_popup_done();
            m_popupGrabber->setClient(0);
        }
    }
}

void ShellSurface::adjustOffset(const QPoint &p)
{
    QPointF offset(p);
    QPointF pos = m_view->requestedPosition();
    m_view->setRequestedPosition(pos + offset);
}

void ShellSurface::requestSize(const QSize &size)
{
    send_configure(WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT, size.width(), size.height());
}

void ShellSurface::shell_surface_destroy_resource(Resource *)
{
    if (m_popupGrabber)
        m_popupGrabber->removePopup(this);

    delete this;
}

void ShellSurface::shell_surface_move(Resource *resource,
                struct wl_resource *input_device_super,
                uint32_t time)
{
    Q_UNUSED(resource);
    Q_UNUSED(time);

    if (m_resizeGrabber || m_moveGrabber) {
        qDebug() << "invalid state";
        return;
    }

    QWaylandInputDevicePrivate *input_device = QWaylandInputDevicePrivate::fromSeatResource(input_device_super);
    QWaylandPointer *pointer = input_device->pointerDevice();

    m_moveGrabber = new ShellSurfaceMoveGrabber(this, pointer->currentSpacePosition() - m_view->requestedPosition());

    pointer->startGrab(m_moveGrabber);
}

void ShellSurface::shell_surface_resize(Resource *resource,
                  struct wl_resource *input_device_super,
                  uint32_t time,
                  uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(time);

    if (m_moveGrabber || m_resizeGrabber) {
        qDebug() << "invalid state2";
        return;
    }

    m_resizeGrabber = new ShellSurfaceResizeGrabber(this);

    QWaylandInputDevicePrivate *input_device = QWaylandInputDevicePrivate::fromSeatResource(input_device_super);
    QWaylandPointer *pointer = input_device->pointerDevice();

    m_resizeGrabber->point = pointer->currentSpacePosition();
    m_resizeGrabber->resize_edges = static_cast<wl_shell_surface_resize>(edges);
    m_resizeGrabber->width = m_view->surface()->size().width();
    m_resizeGrabber->height = m_view->surface()->size().height();

    pointer->startGrab(m_resizeGrabber);
}

void ShellSurface::shell_surface_set_toplevel(Resource *resource)
{
    Q_UNUSED(resource);
    setTransientParent(0);
    setTransientOffset(QPointF(0, 0));

    setSurfaceType(QWaylandSurface::Toplevel);

    m_surface->setVisibility(QWindow::Windowed);
}

void ShellSurface::shell_surface_set_transient(Resource *resource,
                      struct wl_resource *parent_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(resource);
    Q_UNUSED(flags);
    QWaylandSurface *parent_surface = QWaylandSurface::fromResource(parent_surface_resource);
    setTransientParent(parent_surface);
    setTransientOffset(QPointF(x, y));
    if (flags & WL_SHELL_SURFACE_TRANSIENT_INACTIVE)
        m_transientInactive = true;
    else
        m_transientInactive = false;

    setSurfaceType(QWaylandSurface::Transient);

    m_surface->setVisibility(QWindow::AutomaticVisibility);
}

void ShellSurface::shell_surface_set_fullscreen(Resource *resource,
                       uint32_t method,
                       uint32_t framerate,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);
    Q_UNUSED(method);
    Q_UNUSED(framerate);
    QWaylandOutput *output = output_resource
            ? QWaylandOutput::fromResource(output_resource)
            : Q_NULLPTR;
    if (!output) {
        // Look for an output that can contain this surface
        Q_FOREACH (QWaylandOutput *curOutput, m_surface->compositor()->primaryOutputSpace()->outputs()) {
            if (curOutput->geometry().size().width() >= m_surface->size().width() &&
                    curOutput->geometry().size().height() >= m_surface->size().height()) {
                output = curOutput;
                break;
            }
        }
    }
    if (!output) {
        qWarning() << "Unable to resize surface full screen, cannot determine output";
        return;
    }
    QSize outputSize = output->geometry().size();

    m_view->setRequestedPosition(output->geometry().topLeft());
    send_configure(resize_bottom_right, outputSize.width(), outputSize.height());

    m_surface->setVisibility(QWindow::FullScreen);
}

void ShellSurface::shell_surface_set_popup(Resource *resource, wl_resource *input_device, uint32_t serial, wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_UNUSED(input_device);
    Q_UNUSED(flags);

    QWaylandInputDevicePrivate *input = QWaylandInputDevicePrivate::fromSeatResource(input_device);
    m_popupGrabber = m_shell->getPopupGrabber(input->q_func());

    m_popupSerial = serial;
    setTransientParent(QWaylandSurface::fromResource(parent));
    setTransientOffset(QPointF(x, y));

    setSurfaceType(QWaylandSurface::Popup);

    m_surface->setVisibility(QWindow::AutomaticVisibility);
}

void ShellSurface::shell_surface_set_maximized(Resource *resource,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);

    QWaylandOutput *output = output_resource
            ? QWaylandOutput::fromResource(output_resource)
            : Q_NULLPTR;
    if (!output) {
        // Look for an output that can contain this surface
        Q_FOREACH (QWaylandOutput *curOutput, m_surface->compositor()->primaryOutputSpace()->outputs()) {
            if (curOutput->geometry().size().width() >= m_surface->size().width() &&
                    curOutput->geometry().size().height() >= m_surface->size().height()) {
                output = curOutput;
                break;
            }
        }
    }
    if (!output) {
        qWarning() << "Unable to maximize surface, cannot determine output";
        return;
    }
    QSize outputSize = output->availableGeometry().size();

    m_view->setRequestedPosition(output->availableGeometry().topLeft());
    send_configure(resize_bottom_right, outputSize.width(), outputSize.height());

    m_surface->setVisibility(QWindow::Maximized);
}

void ShellSurface::shell_surface_pong(Resource *resource,
                        uint32_t serial)
{
    Q_UNUSED(resource);
    if (m_pings.remove(serial))
        emit m_surface->waylandSurface()->pong();
    else
        qWarning("Received an unexpected pong!");
}

void ShellSurface::shell_surface_set_title(Resource *resource,
                             const QString &title)
{
    Q_UNUSED(resource);
    m_surface->setTitle(title);
}

void ShellSurface::shell_surface_set_class(Resource *resource,
                             const QString &className)
{
    Q_UNUSED(resource);
    m_surface->setClassName(className);
}

ShellSurfaceGrabber::ShellSurfaceGrabber(ShellSurface *shellSurface)
    : QWaylandPointerGrabber()
    , shell_surface(shellSurface)
{
}

ShellSurfaceGrabber::~ShellSurfaceGrabber()
{
}

ShellSurfaceResizeGrabber::ShellSurfaceResizeGrabber(ShellSurface *shellSurface)
    : ShellSurfaceGrabber(shellSurface)
{
}

void ShellSurfaceResizeGrabber::focus()
{
}

void ShellSurfaceResizeGrabber::motion(uint32_t time)
{
    Q_UNUSED(time);

    int width_delta = point.x() - pointer->currentSpacePosition().x();
    int height_delta = point.y() - pointer->currentSpacePosition().y();

    int new_height = height;
    if (resize_edges & WL_SHELL_SURFACE_RESIZE_TOP)
        new_height = qMax(new_height + height_delta, 1);
    else if (resize_edges & WL_SHELL_SURFACE_RESIZE_BOTTOM)
        new_height = qMax(new_height - height_delta, 1);

    int new_width = width;
    if (resize_edges & WL_SHELL_SURFACE_RESIZE_LEFT)
        new_width = qMax(new_width + width_delta, 1);
    else if (resize_edges & WL_SHELL_SURFACE_RESIZE_RIGHT)
        new_width = qMax(new_width - width_delta, 1);

    shell_surface->sendConfigure(resize_edges, new_width, new_height);
}

void ShellSurfaceResizeGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        pointer->endGrab();
        shell_surface->resetResizeGrabber();
        delete this;
    }
}

ShellSurfaceMoveGrabber::ShellSurfaceMoveGrabber(ShellSurface *shellSurface, const QPointF &offset)
    : ShellSurfaceGrabber(shellSurface)
    , m_offset(offset)
{
}

void ShellSurfaceMoveGrabber::focus()
{
}

void ShellSurfaceMoveGrabber::motion(uint32_t time)
{
    Q_UNUSED(time);

    QPointF pos(pointer->currentSpacePosition() - m_offset);
    shell_surface->m_view->setRequestedPosition(pos);
    if (shell_surface->transientParent()) {
        QWaylandView *view = shell_surface->transientParent()->views().first();
        if (view)
            shell_surface->setOffset(pos - view->requestedPosition());
    }

}

void ShellSurfaceMoveGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        pointer->endGrab();
        shell_surface->resetMoveGrabber();
        delete this;
    }
}

ShellSurfacePopupGrabber::ShellSurfacePopupGrabber(QWaylandInputDevice *inputDevice)
    : QWaylandDefaultPointerGrabber()
    , m_inputDevice(inputDevice)
    , m_client(0)
    , m_surfaces()
    , m_initialUp(false)
{
}

uint32_t ShellSurfacePopupGrabber::grabSerial() const
{
    return QWaylandInputDevicePrivate::get(m_inputDevice)->pointerDevice()->grabSerial();
}

struct ::wl_client *ShellSurfacePopupGrabber::client() const
{
    return m_client;
}

void ShellSurfacePopupGrabber::setClient(struct ::wl_client *client)
{
    m_client = client;
}

void ShellSurfacePopupGrabber::addPopup(ShellSurface *surface)
{
    if (m_surfaces.isEmpty()) {
        m_client = surface->resource()->client();

        if (QWaylandInputDevicePrivate::get(m_inputDevice)->pointerDevice()->isButtonPressed())
            m_initialUp = false;

        m_surfaces.append(surface);
        QWaylandInputDevicePrivate::get(m_inputDevice)->pointerDevice()->startGrab(this);
    } else {
        m_surfaces.append(surface);
    }
}

void ShellSurfacePopupGrabber::removePopup(ShellSurface *surface)
{
    if (m_surfaces.isEmpty())
        return;

    m_surfaces.removeOne(surface);
    if (m_surfaces.isEmpty())
        QWaylandInputDevicePrivate::get(m_inputDevice)->pointerDevice()->endGrab();
}

void ShellSurfacePopupGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    if (pointer->focusResource()) {
        pointer->sendButton(pointer->focusResource(), time, button, state);
    } else if (state == QtWaylandServer::wl_pointer::button_state_pressed &&
               (m_initialUp || time - pointer->grabTime() > 500) &&
               pointer->currentGrab() == this) {
        pointer->endGrab();
        Q_FOREACH (ShellSurface *surface, m_surfaces) {
            surface->send_popup_done();
        }
        m_surfaces.clear();
    }

    if (state == QtWaylandServer::wl_pointer::button_state_released)
        m_initialUp = true;
}


}

QT_END_NAMESPACE
