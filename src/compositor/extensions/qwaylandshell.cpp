/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "qwaylandshell.h"
#include "qwaylandshell_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandOutputSpace>
#include <QtWaylandCompositor/QWaylandClient>

#include <QtCore/QObject>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QWaylandShellPrivate::QWaylandShellPrivate()
    : QWaylandExtensionTemplatePrivate()
    , wl_shell()
{
}

QWaylandShellSurfacePopupGrabber *QWaylandShellPrivate::getPopupGrabber(QWaylandInputDevice *input)
{
    if (!m_popupGrabber.contains(input))
        m_popupGrabber.insert(input, new QWaylandShellSurfacePopupGrabber(input));

    return m_popupGrabber.value(input);
}

void QWaylandShellPrivate::shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface_res)
{
    Q_Q(QWaylandShell);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surface_res);
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    emit q_func()->createShellSurface(surface, QWaylandClient::fromWlClient(compositor, resource->client()), id);
}

QWaylandShellSurfacePrivate::QWaylandShellSurfacePrivate()
    : QWaylandExtensionTemplatePrivate()
    , wl_shell_surface()
    , m_shell(Q_NULLPTR)
    , m_surface(Q_NULLPTR)
    , m_view(0)
    , m_resizeGrabber(0)
    , m_moveGrabber(0)
    , m_popupGrabber(0)
    , m_popupSerial()
    , m_surfaceType(QWaylandShellSurface::None)
    , m_transientInactive(false)
    , m_transientParent(0)
    , m_transientOffset()
{
}

QWaylandShellSurfacePrivate::~QWaylandShellSurfacePrivate()
{
    QPointer<QObject> view(m_view);
    if (m_view->renderObject())
        delete m_view->renderObject();
    delete view.data();
}

void QWaylandShellSurfacePrivate::ping()
{
    uint32_t serial = m_surface->compositor()->nextSerial();
    ping(serial);
}

void QWaylandShellSurfacePrivate::ping(uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(serial);
}

void QWaylandShellSurfacePrivate::setSurfaceType(QWaylandShellSurface::SurfaceType type)
{
    Q_Q(QWaylandShellSurface);
    if (m_surfaceType == type)
        return;

    m_surfaceType = type;
    emit q->surfaceTypeChanged();
}

void QWaylandShellSurfacePrivate::resetResizeGrabber()
{
    m_resizeGrabber = 0;
}

void QWaylandShellSurfacePrivate::resetMoveGrabber()
{
    m_moveGrabber = 0;
}

void QWaylandShellSurfacePrivate::shell_surface_destroy_resource(Resource *)
{
    Q_Q(QWaylandShellSurface);
    if (m_popupGrabber)
        m_popupGrabber->removePopup(q);

    delete q;
}

void QWaylandShellSurfacePrivate::shell_surface_move(Resource *resource,
                struct wl_resource *input_device_super,
                uint32_t time)
{
    Q_UNUSED(resource);
    Q_UNUSED(time);

    Q_Q(QWaylandShellSurface);
    if (!m_view)
        return;
    if (m_resizeGrabber || m_moveGrabber) {
        return;
    }

    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(input_device_super);
    QWaylandPointer *pointer = input_device->pointer();

    m_moveGrabber = new QWaylandShellSurfaceMoveGrabber(q, pointer->currentSpacePosition() - m_view->requestedPosition());

    pointer->startGrab(m_moveGrabber);
}

void QWaylandShellSurfacePrivate::shell_surface_resize(Resource *resource,
                  struct wl_resource *input_device_super,
                  uint32_t time,
                  uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(time);
    Q_Q(QWaylandShellSurface);

    if (m_moveGrabber || m_resizeGrabber) {
        return;
    }

    m_resizeGrabber = new QWaylandShellSurfaceResizeGrabber(q);

    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(input_device_super);
    QWaylandPointer *pointer = input_device->pointer();

    m_resizeGrabber->point = pointer->currentSpacePosition();
    m_resizeGrabber->resize_edges = static_cast<wl_shell_surface_resize>(edges);
    m_resizeGrabber->width = m_surface->size().width();
    m_resizeGrabber->height = m_surface->size().height();

    pointer->startGrab(m_resizeGrabber);
}

void QWaylandShellSurfacePrivate::shell_surface_set_toplevel(Resource *resource)
{
    Q_UNUSED(resource);
    m_transientParent = 0;
    m_transientOffset = QPointF();
    setSurfaceType(QWaylandShellSurface::Toplevel);
}

void QWaylandShellSurfacePrivate::shell_surface_set_transient(Resource *resource,
                      struct wl_resource *parent_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(resource);
    Q_UNUSED(flags);
    QWaylandSurface *parent_surface = QWaylandSurface::fromResource(parent_surface_resource);
    m_transientParent = parent_surface;
    m_transientOffset= QPointF(x, y);
    if (flags & WL_SHELL_SURFACE_TRANSIENT_INACTIVE)
        m_transientInactive = true;
    else
        m_transientInactive = false;

    setSurfaceType(QWaylandShellSurface::Transient);
}

void QWaylandShellSurfacePrivate::shell_surface_set_fullscreen(Resource *resource,
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
        Q_FOREACH (QWaylandOutput *curOutput, m_surface->compositor()->defaultOutputSpace()->outputs()) {
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

    if (m_view)
        m_view->setRequestedPosition(output->geometry().topLeft());
    send_configure(resize_bottom_right, outputSize.width(), outputSize.height());

}

void QWaylandShellSurfacePrivate::shell_surface_set_popup(Resource *resource, wl_resource *input_device, uint32_t serial, wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_UNUSED(input_device);
    Q_UNUSED(flags);

    QWaylandInputDevice *input = QWaylandInputDevice::fromSeatResource(input_device);
    m_popupGrabber = QWaylandShellPrivate::get(m_shell)->getPopupGrabber(input);

    m_popupSerial = serial;
    m_transientParent = QWaylandSurface::fromResource(parent);
    m_transientOffset = m_transientParent ? QPointF(x,y) : QPointF();

    setSurfaceType(QWaylandShellSurface::Popup);

}

void QWaylandShellSurfacePrivate::shell_surface_set_maximized(Resource *resource,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);

    QWaylandOutput *output = output_resource
            ? QWaylandOutput::fromResource(output_resource)
            : Q_NULLPTR;
    if (!output) {
        // Look for an output that can contain this surface
        Q_FOREACH (QWaylandOutput *curOutput, m_surface->compositor()->defaultOutputSpace()->outputs()) {
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

    if (m_view)
        m_view->setRequestedPosition(output->availableGeometry().topLeft());
    send_configure(resize_bottom_right, outputSize.width(), outputSize.height());

}

void QWaylandShellSurfacePrivate::shell_surface_pong(Resource *resource,
                        uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandShellSurface);
    if (m_pings.remove(serial))
        emit q->pong();
    else
        qWarning("Received an unexpected pong!");
}

void QWaylandShellSurfacePrivate::shell_surface_set_title(Resource *resource,
                             const QString &title)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandShellSurface);
    m_title = title;
    emit q->titleChanged();
}

void QWaylandShellSurfacePrivate::shell_surface_set_class(Resource *resource,
                             const QString &className)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandShellSurface);
    m_className = className;
    emit q->classNameChanged();
}

QWaylandShellSurfaceGrabber::QWaylandShellSurfaceGrabber(QWaylandShellSurface *shellSurface)
    : QWaylandPointerGrabber()
    , shell_surface(shellSurface)
{
}

QWaylandShellSurfaceGrabber::~QWaylandShellSurfaceGrabber()
{
}

QWaylandShellSurfaceResizeGrabber::QWaylandShellSurfaceResizeGrabber(QWaylandShellSurface *shellSurface)
    : QWaylandShellSurfaceGrabber(shellSurface)
{
}

void QWaylandShellSurfaceResizeGrabber::focus()
{
}

void QWaylandShellSurfaceResizeGrabber::motion(uint32_t time)
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

    QWaylandShellSurfacePrivate::get(shell_surface)->send_configure(resize_edges, new_width, new_height);
}

void QWaylandShellSurfaceResizeGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        pointer->endGrab();
        QWaylandShellSurfacePrivate::get(shell_surface)->resetResizeGrabber();
        delete this;
    }
}

QWaylandShellSurfaceMoveGrabber::QWaylandShellSurfaceMoveGrabber(QWaylandShellSurface *shellSurface, const QPointF &offset)
    : QWaylandShellSurfaceGrabber(shellSurface)
    , m_offset(offset)
{
}

void QWaylandShellSurfaceMoveGrabber::focus()
{
}

void QWaylandShellSurfaceMoveGrabber::motion(uint32_t time)
{
    Q_UNUSED(time);

    QPointF pos(pointer->currentSpacePosition() - m_offset);
    if (shell_surface->view())
        shell_surface->view()->setRequestedPosition(pos);
    if (shell_surface->transientParent()) {
        QWaylandView *view = shell_surface->transientParent()->views().first();
        if (view)
            QWaylandShellSurfacePrivate::get(shell_surface)->setOffset(pos - view->requestedPosition());
    }

}

void QWaylandShellSurfaceMoveGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time)

    if (button == Qt::LeftButton && !state) {
        pointer->endGrab();
        QWaylandShellSurfacePrivate::get(shell_surface)->resetMoveGrabber();
        delete this;
    }
}

QWaylandShellSurfacePopupGrabber::QWaylandShellSurfacePopupGrabber(QWaylandInputDevice *inputDevice)
    : QWaylandDefaultPointerGrabber()
    , m_inputDevice(inputDevice)
    , m_client(0)
    , m_surfaces()
    , m_initialUp(false)
{
}

void QWaylandShellSurfacePopupGrabber::addPopup(QWaylandShellSurface *surface)
{
    if (m_surfaces.isEmpty()) {
        m_client = surface->surface()->client()->client();

        if (m_inputDevice->pointer()->isButtonPressed())
            m_initialUp = false;

        m_surfaces.append(surface);
        m_inputDevice->pointer()->startGrab(this);
    } else {
        m_surfaces.append(surface);
    }
}

void QWaylandShellSurfacePopupGrabber::removePopup(QWaylandShellSurface *surface)
{
    if (m_surfaces.isEmpty())
        return;

    m_surfaces.removeOne(surface);
    if (m_surfaces.isEmpty())
        m_inputDevice->pointer()->endGrab();
}

void QWaylandShellSurfacePopupGrabber::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    if (pointer->focusResource()) {
        pointer->sendButton(pointer->focusResource(), time, button, state);
    } else if (state == QtWaylandServer::wl_pointer::button_state_pressed &&
               (m_initialUp || time - pointer->grabTime() > 500) &&
               pointer->currentGrab() == this) {
        pointer->endGrab();
        Q_FOREACH (QWaylandShellSurface *surface, m_surfaces) {
            QWaylandShellSurfacePrivate::get(surface)->send_popup_done();
        }
        m_surfaces.clear();
    }

    if (state == QtWaylandServer::wl_pointer::button_state_released)
        m_initialUp = true;
}

QWaylandShell::QWaylandShell()
    : QWaylandExtensionTemplate<QWaylandShell>(*new QWaylandShellPrivate())
{ }

QWaylandShell::QWaylandShell(QWaylandCompositor *compositor)
    : QWaylandExtensionTemplate<QWaylandShell>(compositor, *new QWaylandShellPrivate())
{ }

void QWaylandShell::initialize()
{
    Q_D(QWaylandShell);
    QWaylandExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandShell";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *QWaylandShell::interface()
{
    return QWaylandShellPrivate::interface();
}

QByteArray QWaylandShell::interfaceName()
{
    return QWaylandShellPrivate::interfaceName();
}

QWaylandShellSurface::QWaylandShellSurface()
    : QWaylandExtensionTemplate<QWaylandShellSurface>(*new QWaylandShellSurfacePrivate)
{
}

QWaylandShellSurface::QWaylandShellSurface(QWaylandShell *shell, QWaylandSurface *surface, QWaylandView *view, QWaylandClient *client, uint id)
    : QWaylandExtensionTemplate<QWaylandShellSurface>(*new QWaylandShellSurfacePrivate)
{
    initialize(shell, surface, view, client, id);
}

void QWaylandShellSurface::initialize(QWaylandShell *shell, QWaylandSurface *surface, QWaylandView *view, QWaylandClient *client, uint id)
{
    Q_D(QWaylandShellSurface);
    d->m_shell = shell;
    d->m_surface = surface;
    d->m_view = view;
    d->init(client->client(), id, 1);
    connect(surface, &QWaylandSurface::mappedChanged, this, &QWaylandShellSurface::mappedChanged);
    connect(surface, &QWaylandSurface::offsetForNextFrame, this, &QWaylandShellSurface::adjustOffset);
    setExtensionContainer(surface);
    QWaylandExtension::initialize();
}
void QWaylandShellSurface::initialize()
{
    QWaylandExtensionTemplate::initialize();
}

const struct wl_interface *QWaylandShellSurface::interface()
{
    return QWaylandShellSurfacePrivate::interface();
}

QByteArray QWaylandShellSurface::interfaceName()
{
    return QWaylandShellSurfacePrivate::interfaceName();
}

QWaylandShellSurface::SurfaceType QWaylandShellSurface::surfaceType() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_surfaceType;
}

QWaylandView *QWaylandShellSurface::view() const
{
    Q_D(const QWaylandShellSurface);

    return d->m_view;
}

void QWaylandShellSurface::setView(QWaylandView *view)
{
    Q_D(QWaylandShellSurface);
    if (d->m_view == view)
        return;
    d->m_view = view;
    emit viewChanged();
}

QWaylandSurface *QWaylandShellSurface::surface() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_surface;
}

QWaylandSurface *QWaylandShellSurface::transientParent() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_transientParent;
}

QPointF QWaylandShellSurface::transientOffset() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_transientOffset;
}

bool QWaylandShellSurface::isTransientInactive() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_transientInactive;
}

QString QWaylandShellSurface::title() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_title;
}

QString QWaylandShellSurface::className() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_className;
}

void QWaylandShellSurface::mappedChanged()
{
    Q_D(QWaylandShellSurface);
    if (!d->m_surface->isMapped())
        return;

    if (d->m_surfaceType == Popup) {
        if (d->m_surface->isMapped() && d->m_popupGrabber->grabSerial() == d->m_popupSerial) {
            d->m_popupGrabber->addPopup(this);
        } else {
            d->send_popup_done();
            d->m_popupGrabber->setClient(0);
        }
    }
}

void QWaylandShellSurface::adjustOffset(const QPoint &p)
{
    Q_D(QWaylandShellSurface);
    if (!d->m_view)
        return;

    QPointF offset(p);
    QPointF pos = d->m_view->requestedPosition();
    d->m_view->setRequestedPosition(pos + offset);
}

QT_END_NAMESPACE
