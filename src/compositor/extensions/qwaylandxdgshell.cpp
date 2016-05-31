/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qwaylandxdgshell.h"
#include "qwaylandxdgshell_p.h"
#include "qwaylandxdgshellintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandSurfaceRole>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtWaylandCompositor/QWaylandInputDevice>

#include <QtCore/QObject>

#include <algorithm>

QT_BEGIN_NAMESPACE

QWaylandSurfaceRole QWaylandXdgSurfacePrivate::s_role("xdg_surface");
QWaylandSurfaceRole QWaylandXdgPopupPrivate::s_role("xdg_popup");

QWaylandXdgShellPrivate::QWaylandXdgShellPrivate()
    : QWaylandCompositorExtensionPrivate()
    , xdg_shell()
{
}

void QWaylandXdgShellPrivate::ping(Resource *resource, uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(resource->handle, serial);
}

void QWaylandXdgShellPrivate::registerSurface(QWaylandXdgSurface *xdgSurface)
{
    m_xdgSurfaces.insert(xdgSurface->surface()->client()->client(), xdgSurface);
}

void QWaylandXdgShellPrivate::unregisterXdgSurface(QWaylandXdgSurface *xdgSurface)
{
    auto xdgSurfacePrivate = QWaylandXdgSurfacePrivate::get(xdgSurface);
    if (!m_xdgSurfaces.remove(xdgSurfacePrivate->resource()->client(), xdgSurface))
        qWarning("%s Unexpected state. Can't find registered xdg surface\n", Q_FUNC_INFO);
}

void QWaylandXdgShellPrivate::registerXdgPopup(QWaylandXdgPopup *xdgPopup)
{
    m_xdgPopups.insert(xdgPopup->surface()->client()->client(), xdgPopup);
}

void QWaylandXdgShellPrivate::unregisterXdgPopup(QWaylandXdgPopup *xdgPopup)
{
    auto xdgPopupPrivate = QWaylandXdgPopupPrivate::get(xdgPopup);
    if (!m_xdgPopups.remove(xdgPopupPrivate->resource()->client(), xdgPopup))
        qWarning("%s Unexpected state. Can't find registered xdg popup\n", Q_FUNC_INFO);
}

bool QWaylandXdgShellPrivate::isValidPopupParent(QWaylandSurface *parentSurface) const
{
    QWaylandXdgPopup *topmostPopup = topmostPopupForClient(parentSurface->client()->client());
    if (topmostPopup && topmostPopup->surface() != parentSurface) {
        return false;
    }

    QWaylandSurfaceRole *parentRole = parentSurface->role();
    if (parentRole != QWaylandXdgSurface::role() && parentRole != QWaylandXdgPopup::role()) {
        return false;
    }

    return true;
}

QWaylandXdgPopup *QWaylandXdgShellPrivate::topmostPopupForClient(wl_client *client) const
{
    QList<QWaylandXdgPopup *> clientPopups = m_xdgPopups.values(client);
    return clientPopups.empty() ? nullptr : clientPopups.last();
}

QWaylandXdgSurface *QWaylandXdgShellPrivate::xdgSurfaceFromSurface(QWaylandSurface *surface)
{
    Q_FOREACH (QWaylandXdgSurface *xdgSurface, m_xdgSurfaces) {
        if (surface == xdgSurface->surface())
            return xdgSurface;
    }
    return nullptr;
}

void QWaylandXdgShellPrivate::xdg_shell_destroy(Resource *resource)
{
    if (!m_xdgSurfaces.values(resource->client()).empty())
        wl_resource_post_error(resource->handle, XDG_SHELL_ERROR_DEFUNCT_SURFACES,
                               "xdg_shell was destroyed before children");

    wl_resource_destroy(resource->handle);
}

void QWaylandXdgShellPrivate::xdg_shell_get_xdg_surface(Resource *resource, uint32_t id,
                                                        wl_resource *surface_res)
{
    Q_Q(QWaylandXdgShell);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surface_res);

    if (xdgSurfaceFromSurface(surface)) {
        wl_resource_post_error(resource->handle, XDG_SHELL_ERROR_ROLE,
                               "An active xdg_surface already exists for wl_surface@%d",
                               wl_resource_get_id(surface->resource()));
        return;
    }

    if (!surface->setRole(QWaylandXdgSurface::role(), resource->handle, XDG_SHELL_ERROR_ROLE))
        return;

    QWaylandResource xdgSurfaceResource(wl_resource_create(resource->client(), &xdg_surface_interface,
                                                           wl_resource_get_version(resource->handle), id));

    emit q->createXdgSurface(surface, xdgSurfaceResource);

    QWaylandXdgSurface *xdgSurface = QWaylandXdgSurface::fromResource(xdgSurfaceResource.resource());
    if (!xdgSurface) {
        // A QWaylandXdgSurface was not created in response to the createXdgSurface signal, so we
        // create one as fallback here instead.
        xdgSurface = new QWaylandXdgSurface(q, surface, xdgSurfaceResource);
    }

    registerSurface(xdgSurface);
    emit q->xdgSurfaceCreated(xdgSurface);
}

void QWaylandXdgShellPrivate::xdg_shell_use_unstable_version(Resource *resource, int32_t version)
{
    if (xdg_shell::version_current != version) {
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "incompatible version, server is %d, but client wants %d",
                               xdg_shell::version_current, version);
    }
}

void QWaylandXdgShellPrivate::xdg_shell_get_xdg_popup(Resource *resource, uint32_t id,
                                                      wl_resource *surface_res, wl_resource *parent,
                                                      wl_resource *seat, uint32_t serial,
                                                      int32_t x, int32_t y)
{
    Q_UNUSED(serial);
    Q_Q(QWaylandXdgShell);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surface_res);
    QWaylandSurface *parentSurface = QWaylandSurface::fromResource(parent);

    if (!isValidPopupParent(parentSurface)) {
        wl_resource_post_error(resource->handle, XDG_SHELL_ERROR_INVALID_POPUP_PARENT,
                               "the client specified an invalid popup parent surface");
        return;
    }

    if (!surface->setRole(QWaylandXdgPopup::role(), resource->handle, XDG_SHELL_ERROR_ROLE)) {
        return;
    }

    QWaylandResource xdgPopupResource (wl_resource_create(resource->client(), &xdg_popup_interface,
                                                          wl_resource_get_version(resource->handle), id));
    QWaylandInputDevice *inputDevice = QWaylandInputDevice::fromSeatResource(seat);
    QPoint position(x, y);
    emit q->createXdgPopup(surface, parentSurface, inputDevice, position, xdgPopupResource);

    QWaylandXdgPopup *xdgPopup = QWaylandXdgPopup::fromResource(xdgPopupResource.resource());
    if (!xdgPopup) {
        // A QWaylandXdgPopup was not created in response to the createXdgPopup signal, so we
        // create one as fallback here instead.
        xdgPopup = new QWaylandXdgPopup(q, surface, parentSurface, xdgPopupResource);
    }

    registerXdgPopup(xdgPopup);
    emit q->xdgPopupCreated(xdgPopup);
}

void QWaylandXdgShellPrivate::xdg_shell_pong(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgShell);
    if (m_pings.remove(serial))
        emit q->pong(serial);
    else
        qWarning("Received an unexpected pong!");
}

QWaylandXdgSurfacePrivate::QWaylandXdgSurfacePrivate()
    : QWaylandCompositorExtensionPrivate()
    , xdg_surface()
    , m_surface(nullptr)
    , m_parentSurface(nullptr)
    , m_unsetWindowGeometry(true)
    , m_lastAckedConfigure({{}, QSize(0, 0), 0})
{
}

void QWaylandXdgSurfacePrivate::handleFocusLost()
{
    Q_Q(QWaylandXdgSurface);
    QWaylandXdgSurfacePrivate::ConfigureEvent current = lastSentConfigure();
    current.states.removeOne(QWaylandXdgSurface::State::ActivatedState);
    q->sendConfigure(current.size, current.states);
}

void QWaylandXdgSurfacePrivate::handleFocusReceived()
{
    Q_Q(QWaylandXdgSurface);

    QWaylandXdgSurfacePrivate::ConfigureEvent current = lastSentConfigure();
    if (!current.states.contains(QWaylandXdgSurface::State::ActivatedState)) {
        current.states.push_back(QWaylandXdgSurface::State::ActivatedState);
    }

    q->sendConfigure(current.size, current.states);
}

void QWaylandXdgSurfacePrivate::xdg_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);
    QWaylandXdgShellPrivate::get(m_xdgShell)->unregisterXdgSurface(q);
    delete q;
}

void QWaylandXdgSurfacePrivate::xdg_surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandXdgSurfacePrivate::xdg_surface_move(Resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(QWaylandXdgSurface);
    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(seat);
    emit q->startMove(input_device);
}

void QWaylandXdgSurfacePrivate::xdg_surface_resize(Resource *resource, wl_resource *seat,
                                                   uint32_t serial, uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(QWaylandXdgSurface);
    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(seat);
    emit q->startResize(input_device, QWaylandXdgSurface::ResizeEdge(edges));
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_maximized(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);
    emit q->setMaximized();
}

void QWaylandXdgSurfacePrivate::xdg_surface_unset_maximized(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);
    emit q->unsetMaximized();
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_fullscreen(Resource *resource, wl_resource *output_res)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);
    QWaylandOutput *output = output_res ? QWaylandOutput::fromResource(output_res) : nullptr;
    emit q->setFullscreen(output);
}

void QWaylandXdgSurfacePrivate::xdg_surface_unset_fullscreen(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);
    emit q->unsetFullscreen();
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_minimized(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);
    emit q->setMinimized();
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_parent(Resource *resource, wl_resource *parent)
{
    Q_UNUSED(resource);
    QWaylandXdgSurface *parentSurface = nullptr;
    if (parent) {
        parentSurface = static_cast<QWaylandXdgSurfacePrivate *>(
                    QWaylandXdgSurfacePrivate::Resource::fromResource(parent)->xdg_surface_object)->q_func();
    }

    if (m_parentSurface == parentSurface)
        return;

    Q_Q(QWaylandXdgSurface);
    m_parentSurface = parentSurface;
    emit q->parentSurfaceChanged();
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_app_id(Resource *resource, const QString &app_id)
{
    Q_UNUSED(resource);
    if (app_id == m_appId)
        return;
    Q_Q(QWaylandXdgSurface);
    m_appId = app_id;
    emit q->appIdChanged();
}

void QWaylandXdgSurfacePrivate::xdg_surface_show_window_menu(Resource *resource, wl_resource *seat,
                                                             uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    QPoint position(x, y);
    auto inputDevice = QWaylandInputDevice::fromSeatResource(seat);
    Q_Q(QWaylandXdgSurface);
    emit q->showWindowMenu(inputDevice, position);
}

void QWaylandXdgSurfacePrivate::xdg_surface_ack_configure(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgSurface);

    ConfigureEvent config;
    Q_FOREVER {
        if (m_pendingConfigures.empty()) {
            qWarning("Received an unexpected ack_configure!");
            return;
        }

        config = m_pendingConfigures.takeFirst();

        if (config.serial == serial)
            break;
    }

    QVector<uint> changedStates;
    std::set_symmetric_difference(
                m_lastAckedConfigure.states.begin(), m_lastAckedConfigure.states.end(),
                config.states.begin(), config.states.end(),
                std::back_inserter(changedStates));

    m_lastAckedConfigure = config;

    if (!changedStates.empty()) {
        Q_FOREACH (uint state, changedStates) {
            switch (state) {
            case QWaylandXdgSurface::State::MaximizedState:
                emit q->maximizedChanged();
                break;
            case QWaylandXdgSurface::State::FullscreenState:
                emit q->fullscreenChanged();
                break;
            case QWaylandXdgSurface::State::ResizingState:
                emit q->resizingChanged();
                break;
            case QWaylandXdgSurface::State::ActivatedState:
                emit q->activatedChanged();
                break;
            }
        }
        emit q->statesChanged();
    }

    emit q->ackConfigure(serial);
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_title(Resource *resource, const QString &title)
{
    Q_UNUSED(resource);
    if (title == m_title)
        return;
    Q_Q(QWaylandXdgSurface);
    m_title = title;
    emit q->titleChanged();
}

void QWaylandXdgSurfacePrivate::xdg_surface_set_window_geometry(Resource *resource,
                                                                int32_t x, int32_t y,
                                                                int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid (non-positive) dimensions received in set_window_geometry";
        return;
    }

    m_unsetWindowGeometry = false;

    QRect geometry(x, y, width, height);

    Q_Q(QWaylandXdgSurface);
    if ((q->maximized() || q->fullscreen()) && m_lastAckedConfigure.size != geometry.size())
        qWarning() << "Client window geometry did not obey last acked configure";

    if (geometry == m_windowGeometry)
        return;

    m_windowGeometry = geometry;
    emit q->windowGeometryChanged();
}

QWaylandXdgPopupPrivate::QWaylandXdgPopupPrivate()
    : QWaylandCompositorExtensionPrivate()
    , xdg_popup()
    , m_surface(nullptr)
    , m_parentSurface(nullptr)
    , m_xdgShell(nullptr)
{
}

void QWaylandXdgPopupPrivate::xdg_popup_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandXdgPopup);
    QWaylandXdgShellPrivate::get(m_xdgShell)->unregisterXdgPopup(q);
    delete q;
}

void QWaylandXdgPopupPrivate::xdg_popup_destroy(Resource *resource)
{
    //TODO: post error if not topmost popup
    wl_resource_destroy(resource->handle);
}

/*!
 * Constructs a QWaylandXdgShell object.
 */
QWaylandXdgShell::QWaylandXdgShell()
    : QWaylandCompositorExtensionTemplate<QWaylandXdgShell>(*new QWaylandXdgShellPrivate())
{ }

/*!
 * Constructs a QWaylandXdgShell object for the provided \a compositor.
 */
QWaylandXdgShell::QWaylandXdgShell(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandXdgShell>(compositor, *new QWaylandXdgShellPrivate())
{ }

/*!
 * Initializes the shell extension.
 */
void QWaylandXdgShell::initialize()
{
    Q_D(QWaylandXdgShell);
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandXdgShell";
        return;
    }
    d->init(compositor->display(), 1);

    handleDefaultInputDeviceChanged(compositor->defaultInputDevice(), nullptr);

    connect(compositor, &QWaylandCompositor::defaultInputDeviceChanged,
            this, &QWaylandXdgShell::handleDefaultInputDeviceChanged);
}

/*!
 * Returns the Wayland interface for the QWaylandXdgShell.
 */
const struct wl_interface *QWaylandXdgShell::interface()
{
    return QWaylandXdgShellPrivate::interface();
}

QByteArray QWaylandXdgShell::interfaceName()
{
    return QWaylandXdgShellPrivate::interfaceName();
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgSurface::ping()
 *
 * Sends a ping event to the client. If the client replies to the event the
 * \a pong signal will be emitted.
 */

/*!
 * Sends a ping event to the client. If the client replies to the event the
 * \a pong signal will be emitted.
 */
uint QWaylandXdgShell::ping(QWaylandClient *client)
{
    Q_D(QWaylandXdgShell);

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    Q_ASSERT(compositor);

    uint32_t serial = compositor->nextSerial();

    QWaylandXdgShellPrivate::Resource *clientResource = d->resourceMap().value(client->client(), nullptr);
    Q_ASSERT(clientResource);

    d->ping(clientResource, serial);
    return serial;
}

void QWaylandXdgShell::closeAllPopups()
{
    Q_D(QWaylandXdgShell);
    Q_FOREACH (struct wl_client *client, d->m_xdgPopups.keys()) {
        QList<QWaylandXdgPopup *> popups = d->m_xdgPopups.values(client);
        std::reverse(popups.begin(), popups.end());
        Q_FOREACH (QWaylandXdgPopup *currentTopmostPopup, popups) {
            currentTopmostPopup->sendPopupDone();
        }
    }
}

void QWaylandXdgShell::handleDefaultInputDeviceChanged(QWaylandInputDevice *newDevice, QWaylandInputDevice *oldDevice)
{
    if (oldDevice != nullptr) {
        disconnect(oldDevice, &QWaylandInputDevice::keyboardFocusChanged,
                   this, &QWaylandXdgShell::handleFocusChanged);
    }

    if (newDevice != nullptr) {
        connect(newDevice, &QWaylandInputDevice::keyboardFocusChanged,
                this, &QWaylandXdgShell::handleFocusChanged);
    }
}

void QWaylandXdgShell::handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    Q_D(QWaylandXdgShell);

    QWaylandXdgSurface *newXdgSurface = d->xdgSurfaceFromSurface(newSurface);
    QWaylandXdgSurface *oldXdgSurface = d->xdgSurfaceFromSurface(oldSurface);

    if (newXdgSurface)
        QWaylandXdgSurfacePrivate::get(newXdgSurface)->handleFocusReceived();

    if (oldXdgSurface)
        QWaylandXdgSurfacePrivate::get(oldXdgSurface)->handleFocusLost();
}

/*!
 * \class QWaylandXdgSurface
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief An xdg surface providing desktop-style compositor-specific features to a surface.
 *
 * This class is part of the QWaylandXdgShell extension and provides a way to
 * extend the functionality of an existing QWaylandSurface with features
 * specific to desktop-style compositors, such as resizing and moving the
 * surface.
 *
 * It corresponds to the Wayland interface xdg_surface.
 */

/*!
 * Constructs a QWaylandXdgSurface.
 */
QWaylandXdgSurface::QWaylandXdgSurface()
    : QWaylandShellSurfaceTemplate<QWaylandXdgSurface>(*new QWaylandXdgSurfacePrivate)
{
}

/*!
 * Constructs a QWaylandXdgSurface for \a surface and initializes it with the
 * given \a xdgShell, \a surface and \a resource.
 */
QWaylandXdgSurface::QWaylandXdgSurface(QWaylandXdgShell *xdgShell, QWaylandSurface *surface, const QWaylandResource &res)
    : QWaylandShellSurfaceTemplate<QWaylandXdgSurface>(*new QWaylandXdgSurfacePrivate)
{
    initialize(xdgShell, surface, res);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgSurface::initialize(object surface, object client, int id)
 *
 * Initializes the XdgSurface, associating it with the given \a surface,
 * \a client, and \a id.
 */

/*!
 * Initializes the QWaylandXdgSurface, associating it with the given \a xdgShell, \a surface
 * and \a resource.
 */
void QWaylandXdgSurface::initialize(QWaylandXdgShell *xdgShell, QWaylandSurface *surface, const QWaylandResource &resource)
{
    Q_D(QWaylandXdgSurface);
    d->m_xdgShell = xdgShell;
    d->m_surface = surface;
    d->init(resource.resource());
    setExtensionContainer(surface);
    d->m_windowGeometry = QRect(QPoint(0,0), surface->size());
    connect(surface, &QWaylandSurface::sizeChanged, this, &QWaylandXdgSurface::handleSurfaceSizeChanged);
    emit surfaceChanged();
    emit windowGeometryChanged();
    QWaylandCompositorExtension::initialize();
}

/*!
 * \internal
 */
void QWaylandXdgSurface::initialize()
{
    QWaylandCompositorExtension::initialize();
}

QList<int> QWaylandXdgSurface::statesAsInts() const
{
   QList<int> list;
   Q_FOREACH (uint state, states()) {
       list << static_cast<int>(state);
   }
   return list;
}

void QWaylandXdgSurface::handleSurfaceSizeChanged()
{
    Q_D(QWaylandXdgSurface);
    if (d->m_unsetWindowGeometry && d->m_windowGeometry.size() != surface()->size()) {
        // TODO: The unset window geometry should include subsurfaces as well, so this solution
        // won't work too well on those kinds of clients.
        d->m_windowGeometry.setSize(surface()->size());
        emit windowGeometryChanged();
    }
}

/*!
 * \qmlproperty object QtWaylandCompositor::XdgSurface::surface
 *
 * This property holds the surface associated with this XdgSurface.
 */

/*!
 * \property QWaylandXdgSurface::surface
 *
 * This property holds the surface associated with this QWaylandXdgSurface.
 */
QWaylandSurface *QWaylandXdgSurface::surface() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_surface;
}

/*!
 * \qmlproperty object QtWaylandCompositor::XdgSurface::parentSurface
 *
 * This property holds the XdgSurface parent of this XdgSurface.
 */

/*!
 * \property QWaylandXdgSurface::surface
 *
 * This property holds the XdgSurface parent of this XdgSurface.
 */
QWaylandXdgSurface *QWaylandXdgSurface::parentSurface() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_parentSurface;
}

/*!
 * \qmlproperty string QtWaylandCompositor::XdgSurface::title
 *
 * This property holds the title of the XdgSurface.
 */

/*!
 * \property QWaylandXdgSurface::title
 *
 * This property holds the title of the QWaylandXdgSurface.
 */
QString QWaylandXdgSurface::title() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_title;
}

/*!
 * \property QWaylandXdgSurface::appId
 *
 * This property holds the app id of the QWaylandXdgSurface.
 */
QString QWaylandXdgSurface::appId() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_appId;
}

/*!
 * \property QWaylandXdgSurface::windowGeometry
 *
 * This property holds the window geometry of the QWaylandXdgSurface. The window
 * geometry describes the window's visible bounds from the user's perspective.
 * The geometry includes title bars and borders if drawn by the client, but
 * excludes drop shadows. It is meant to be used for aligning and tiling
 * windows.
 */
QRect QWaylandXdgSurface::windowGeometry() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_windowGeometry;
}

/*!
 * \property QWaylandXdgSurface::states
 *
 * This property holds the last states the client acknowledged for this QWaylandXdgSurface.
 */
QVector<uint> QWaylandXdgSurface::states() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_lastAckedConfigure.states;
}

bool QWaylandXdgSurface::maximized() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgSurface::State::MaximizedState);
}

bool QWaylandXdgSurface::fullscreen() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgSurface::State::FullscreenState);
}

bool QWaylandXdgSurface::resizing() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgSurface::State::ResizingState);
}

bool QWaylandXdgSurface::activated() const
{
    Q_D(const QWaylandXdgSurface);
    return d->m_lastAckedConfigure.states.contains(QWaylandXdgSurface::State::ActivatedState);
}

/*!
 * Returns the Wayland interface for the QWaylandXdgSurface.
 */
const wl_interface *QWaylandXdgSurface::interface()
{
    return QWaylandXdgSurfacePrivate::interface();
}

QByteArray QWaylandXdgSurface::interfaceName()
{
    return QWaylandXdgSurfacePrivate::interfaceName();
}

/*!
 * Returns the surface role for the QWaylandXdgSurface.
 */
QWaylandSurfaceRole *QWaylandXdgSurface::role()
{
    return &QWaylandXdgSurfacePrivate::s_role;
}

/*!
 * Returns the QWaylandXdgSurface corresponding to the \a resource.
 */
QWaylandXdgSurface *QWaylandXdgSurface::fromResource(wl_resource *resource)
{
    auto xsResource = QWaylandXdgSurfacePrivate::Resource::fromResource(resource);
    if (!xsResource)
        return nullptr;
    return static_cast<QWaylandXdgSurfacePrivate *>(xsResource->xdg_surface_object)->q_func();
}

QSize QWaylandXdgSurface::sizeForResize(const QSizeF &size, const QPointF &delta,
                                        QWaylandXdgSurface::ResizeEdge edge)
{
    qreal width = size.width();
    qreal height = size.height();
    if (edge & LeftEdge)
        width -= delta.x();
    else if (edge & RightEdge)
        width += delta.x();

    if (edge & TopEdge)
        height -= delta.y();
    else if (edge & BottomEdge)
        height += delta.y();

    return QSizeF(width, height).toSize();
}

/*!
 * \qmlmethod int QtWaylandCompositor::XdgSurface::sendConfigure(size size, List<uint>)
 *
 * Sends a configure event to the client. Known states are enumerated in XdgSurface::State
 */

/*!
 * Sends a configure event to the client. Known states are enumerated in QWaylandXdgSurface::State
 */
uint QWaylandXdgSurface::sendConfigure(const QSize &size, const QVector<uint> &states)
{
    Q_D(QWaylandXdgSurface);
    auto statesBytes = QByteArray::fromRawData((char *)states.data(), states.size() * sizeof(State));
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    Q_ASSERT(compositor);
    uint32_t serial = compositor->nextSerial();
    d->m_pendingConfigures.append(QWaylandXdgSurfacePrivate::ConfigureEvent{states, size, serial});
    d->send_configure(size.width(), size.height(), statesBytes, serial);
    return serial;
}

uint QWaylandXdgSurface::sendConfigure(const QSize &size, const QVector<QWaylandXdgSurface::State> &states)
{
    QVector<uint> asUints;
    Q_FOREACH (QWaylandXdgSurface::State state, states) {
        asUints << state;
    }
    return sendConfigure(size, asUints);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgSurface::sendClose()
 *
 * Sends a close event to the client.
 */

/*!
 * Sends a close event to the client.
 */
void QWaylandXdgSurface::sendClose()
{
    Q_D(QWaylandXdgSurface);
    d->send_close();
}

uint QWaylandXdgSurface::requestMaximized(const QSize &size)
{
    Q_D(QWaylandXdgSurface);
    QWaylandXdgSurfacePrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgSurface::State::MaximizedState))
        conf.states.append(QWaylandXdgSurface::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgSurface::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgSurface::State::ResizingState);

    return sendConfigure(size, conf.states);
}

uint QWaylandXdgSurface::requestUnMaximized(const QSize &size)
{
    Q_D(QWaylandXdgSurface);
    QWaylandXdgSurfacePrivate::ConfigureEvent conf = d->lastSentConfigure();

    conf.states.removeOne(QWaylandXdgSurface::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgSurface::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgSurface::State::ResizingState);

    return sendConfigure(size, conf.states);
}

uint QWaylandXdgSurface::requestFullscreen(const QSize &size)
{
    Q_D(QWaylandXdgSurface);
    QWaylandXdgSurfacePrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgSurface::State::FullscreenState))
        conf.states.append(QWaylandXdgSurface::State::FullscreenState);
    conf.states.removeOne(QWaylandXdgSurface::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgSurface::State::ResizingState);

    return sendConfigure(size, conf.states);
}

uint QWaylandXdgSurface::requestResizing(const QSize &maxSize)
{
    Q_D(QWaylandXdgSurface);
    QWaylandXdgSurfacePrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(QWaylandXdgSurface::State::ResizingState))
        conf.states.append(QWaylandXdgSurface::State::ResizingState);
    conf.states.removeOne(QWaylandXdgSurface::State::MaximizedState);
    conf.states.removeOne(QWaylandXdgSurface::State::FullscreenState);

    return sendConfigure(maxSize, conf.states);
}

QWaylandQuickShellIntegration *QWaylandXdgSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    return new QtWayland::XdgShellIntegration(item);
}

/*!
 * \class QWaylandXdgPopup
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief An xdg popup providing menus for an xdg surface
 *
 * This class is part of the QWaylandXdgShell extension and provides a way to
 * extend the functionality of an existing QWaylandSurface with features
 * specific to desktop-style menus for an xdg surface.
 *
 * It corresponds to the Wayland interface xdg_popup.
 */

/*!
 * Constructs a QWaylandXdgPopup.
 */
QWaylandXdgPopup::QWaylandXdgPopup()
    : QWaylandCompositorExtensionTemplate<QWaylandXdgPopup>(*new QWaylandXdgPopupPrivate)
{
}

/*!
 * Constructs a QWaylandXdgPopup for \a surface and initializes it with the
 * given \a parentSurface and \a resource.
 */
QWaylandXdgPopup::QWaylandXdgPopup(QWaylandXdgShell *xdgShell, QWaylandSurface *surface,
                                   QWaylandSurface *parentSurface, const QWaylandResource &resource)
    : QWaylandCompositorExtensionTemplate<QWaylandXdgPopup>(*new QWaylandXdgPopupPrivate)
{
    initialize(xdgShell, surface, parentSurface, resource);
}

/*!
 * \qmlmethod void QtWaylandCompositor::XdgPopup::initialize(object surface, object parentSurface, object resource)
 *
 * Initializes the xdg popup, associating it with the given \a shell, \a surface,
 * \a parentSurface and \a resource.
 */

/*!
 * Initializes the QWaylandXdgPopup, associating it with the given \a shell \a surface,
 * \a parentSurface and \a resource.
 */
void QWaylandXdgPopup::initialize(QWaylandXdgShell *shell, QWaylandSurface *surface,
                                  QWaylandSurface *parentSurface, const QWaylandResource &resource)
{
    Q_D(QWaylandXdgPopup);
    d->m_surface = surface;
    d->m_parentSurface = parentSurface;
    d->m_xdgShell = shell;
    d->init(resource.resource());
    setExtensionContainer(surface);
    emit surfaceChanged();
    emit parentSurfaceChanged();
    QWaylandCompositorExtension::initialize();
}

/*!
 * \qmlproperty object QtWaylandCompositor::XdgPopup::surface
 *
 * This property holds the surface associated with this XdgPopup.
 */

/*!
 * \property QWaylandXdgPopup::surface
 *
 * This property holds the surface associated with this QWaylandXdgPopup.
 */
QWaylandSurface *QWaylandXdgPopup::surface() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_surface;
}

/*!
 * \qmlproperty object QtWaylandCompositor::XdgPopup::parentSurface
 *
 * This property holds the surface associated with the parent of this XdgPopup.
 */

/*!
 * \property QWaylandXdgPopup::parentSurface
 *
 * This property holds the surface associated with the parent of this
 * QWaylandXdgPopup.
 */
QWaylandSurface *QWaylandXdgPopup::parentSurface() const
{
    Q_D(const QWaylandXdgPopup);
    return d->m_parentSurface;
}

/*!
 * \internal
 */
void QWaylandXdgPopup::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
}

/*!
 * Returns the Wayland interface for the QWaylandXdgPopup.
 */
const wl_interface *QWaylandXdgPopup::interface()
{
    return QWaylandXdgPopupPrivate::interface();
}

QByteArray QWaylandXdgPopup::interfaceName()
{
    return QWaylandXdgPopupPrivate::interfaceName();
}

/*!
 * Returns the surface role for the QWaylandXdgPopup.
 */
QWaylandSurfaceRole *QWaylandXdgPopup::role()
{
    return &QWaylandXdgPopupPrivate::s_role;
}

QWaylandXdgPopup *QWaylandXdgPopup::fromResource(wl_resource *resource)
{
    auto popupResource = QWaylandXdgPopupPrivate::Resource::fromResource(resource);
    if (!popupResource)
        return nullptr;
    return static_cast<QWaylandXdgPopupPrivate *>(popupResource->xdg_popup_object)->q_func();
}

void QWaylandXdgPopup::sendPopupDone()
{
    Q_D(QWaylandXdgPopup);
    d->send_popup_done();
}

QT_END_NAMESPACE
