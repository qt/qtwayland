/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "qwaylandwlshell.h"
#include "qwaylandwlshell_p.h"

#include "qwaylandwlshellintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandClient>

#include <QtCore/QObject>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QWaylandSurfaceRole QWaylandWlShellSurfacePrivate::s_role("wl_shell_surface");

QWaylandWlShellPrivate::QWaylandWlShellPrivate()
    : QWaylandCompositorExtensionPrivate()
    , wl_shell()
{
}

void QWaylandWlShellPrivate::shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface_res)
{
    Q_Q(QWaylandWlShell);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surface_res);

    QWaylandResource shellSurfaceResource(wl_resource_create(resource->client(), &wl_shell_surface_interface,
                                                             wl_resource_get_version(resource->handle), id));

    // XXX FIXME
    // The role concept was formalized in wayland 1.7, so that release adds one error
    // code for each interface that implements a role, and we are supposed to pass here
    // the newly constructed resource and the correct error code so that if setting the
    // role fails, a proper error can be sent to the client.
    // However we're still using wayland 1.4, which doesn't have interface specific role
    // errors, so the best we can do is to use wl_display's object_id error.
    wl_resource *displayRes = wl_client_get_object(resource->client(), 1);
    if (!surface->setRole(QWaylandWlShellSurface::role(), displayRes, WL_DISPLAY_ERROR_INVALID_OBJECT))
        return;

    emit q->createShellSurface(surface, shellSurfaceResource);

    QWaylandWlShellSurface *shellSurface = QWaylandWlShellSurface::fromResource(shellSurfaceResource.resource());
    if (!shellSurface) {
        // A QWaylandShellSurface was not created in response to the createShellSurface signal
        // we create one as fallback here instead.
        shellSurface = new QWaylandWlShellSurface(q, surface, shellSurfaceResource);
    }

    emit q->shellSurfaceCreated(shellSurface);
}

QWaylandWlShellSurfacePrivate::QWaylandWlShellSurfacePrivate()
    : QWaylandCompositorExtensionPrivate()
    , wl_shell_surface()
    , m_shell(Q_NULLPTR)
    , m_surface(Q_NULLPTR)
    , m_focusPolicy(QWaylandWlShellSurface::DefaultFocus)
{
}

QWaylandWlShellSurfacePrivate::~QWaylandWlShellSurfacePrivate()
{
}

void QWaylandWlShellSurfacePrivate::ping(uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(serial);
}

void QWaylandWlShellSurfacePrivate::shell_surface_destroy_resource(Resource *)
{
    Q_Q(QWaylandWlShellSurface);

    delete q;
}

void QWaylandWlShellSurfacePrivate::shell_surface_move(Resource *resource,
                struct wl_resource *input_device_super,
                uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(QWaylandWlShellSurface);
    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(input_device_super);
    emit q->startMove(input_device);
}

void QWaylandWlShellSurfacePrivate::shell_surface_resize(Resource *resource,
                  struct wl_resource *input_device_super,
                  uint32_t serial,
                  uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandWlShellSurface);

    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(input_device_super);
    emit q->startResize(input_device, QWaylandWlShellSurface::ResizeEdge(edges));
}

void QWaylandWlShellSurfacePrivate::shell_surface_set_toplevel(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandWlShellSurface);
    setFocusPolicy(QWaylandWlShellSurface::DefaultFocus);
    emit q->setDefaultToplevel();
}

void QWaylandWlShellSurfacePrivate::shell_surface_set_transient(Resource *resource,
                      struct wl_resource *parent_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(resource);
    Q_Q(QWaylandWlShellSurface);
    QWaylandSurface *parent_surface = QWaylandSurface::fromResource(parent_surface_resource);
    QWaylandWlShellSurface::FocusPolicy focusPolicy =
        flags & WL_SHELL_SURFACE_TRANSIENT_INACTIVE ? QWaylandWlShellSurface::NoKeyboardFocus
                                                    : QWaylandWlShellSurface::DefaultFocus;
    setFocusPolicy(focusPolicy);
    emit q->setTransient(parent_surface, QPoint(x,y), focusPolicy);
}

void QWaylandWlShellSurfacePrivate::shell_surface_set_fullscreen(Resource *resource,
                       uint32_t method,
                       uint32_t framerate,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);
    Q_UNUSED(method);
    Q_UNUSED(framerate);
    Q_Q(QWaylandWlShellSurface);
    setFocusPolicy(QWaylandWlShellSurface::DefaultFocus);
    QWaylandOutput *output = output_resource
            ? QWaylandOutput::fromResource(output_resource)
            : Q_NULLPTR;
    emit q->setFullScreen(QWaylandWlShellSurface::FullScreenMethod(method), framerate, output);
}

void QWaylandWlShellSurfacePrivate::shell_surface_set_popup(Resource *resource, wl_resource *input_device, uint32_t serial, wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_UNUSED(flags);
    Q_Q(QWaylandWlShellSurface);
    setFocusPolicy(QWaylandWlShellSurface::DefaultFocus);
    QWaylandInputDevice *input = QWaylandInputDevice::fromSeatResource(input_device);
    QWaylandSurface *parentSurface = QWaylandSurface::fromResource(parent);
    emit q->setPopup(input, parentSurface, QPoint(x,y));

}

void QWaylandWlShellSurfacePrivate::shell_surface_set_maximized(Resource *resource,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandWlShellSurface);
    setFocusPolicy(QWaylandWlShellSurface::DefaultFocus);
    QWaylandOutput *output = output_resource
            ? QWaylandOutput::fromResource(output_resource)
            : Q_NULLPTR;
    emit q->setMaximized(output);
}

void QWaylandWlShellSurfacePrivate::shell_surface_pong(Resource *resource,
                        uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandWlShellSurface);
    if (m_pings.remove(serial))
        emit q->pong();
    else
        qWarning("Received an unexpected pong!");
}

void QWaylandWlShellSurfacePrivate::shell_surface_set_title(Resource *resource,
                             const QString &title)
{
    Q_UNUSED(resource);
    if (title == m_title)
        return;
    Q_Q(QWaylandWlShellSurface);
    m_title = title;
    emit q->titleChanged();
}

void QWaylandWlShellSurfacePrivate::shell_surface_set_class(Resource *resource,
                             const QString &className)
{
    Q_UNUSED(resource);
    if (className == m_className)
        return;
    Q_Q(QWaylandWlShellSurface);
    m_className = className;
    emit q->classNameChanged();
}

/*!
 * \qmltype WlShell
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \brief Extension for desktop-style user interfaces.
 *
 * The WlShell extension provides a way to assiociate a \l{ShellSurface}
 * with a regular Wayland surface. Using the shell surface interface, the client
 * can request that the surface is resized, moved, and so on.
 *
 * WlShell corresponds to the Wayland interface wl_shell.
 *
 * To provide the functionality of the shell extension in a compositor, create
 * an instance of the WlShell component and add it to the list of extensions
 * supported by the compositor:
 * \code
 * import QtWayland.Compositor 1.0
 *
 * WaylandCompositor {
 *     extensions: WlShell {
 *         // ...
 *     }
 * }
 * \endcode
 */

/*!
 * \class QWaylandWlShell
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief Extension for desktop-style user interfaces.
 *
 * The QWaylandWlShell extension provides a way to assiociate a QWaylandWlShellSurface with
 * a regular Wayland surface. Using the shell surface interface, the client
 * can request that the surface is resized, moved, and so on.
 *
 * WlShell corresponds to the Wayland interface wl_shell.
 */

/*!
 * Constructs a QWaylandWlShell object.
 */
QWaylandWlShell::QWaylandWlShell()
    : QWaylandCompositorExtensionTemplate<QWaylandWlShell>(*new QWaylandWlShellPrivate())
{ }

/*!
 * Constructs a QWaylandWlShell object for the provided \a compositor.
 */
QWaylandWlShell::QWaylandWlShell(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandWlShell>(compositor, *new QWaylandWlShellPrivate())
{ }


/*!
 * Initializes the WlShell extension.
 */
void QWaylandWlShell::initialize()
{
    Q_D(QWaylandWlShell);
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandWlShell";
        return;
    }
    d->init(compositor->display(), 1);
}

/*!
 * Returns the Wayland interface for the QWaylandWlShell.
 */
const struct wl_interface *QWaylandWlShell::interface()
{
    return QWaylandWlShellPrivate::interface();
}

/*!
 * \qmlsignal void QtWaylandCompositor::WlShell::createShellSurface(object surface, object client, int id)
 *
 * This signal is emitted when the \a client has requested a wl_shell_surface to be associated
 * with \a surface and be assigned the given \a id. The handler for this signal is
 * expected to create the shell surface and initialize it within the scope of the
 * signal emission.
 */

/*!
 * \fn void QWaylandWlShell::createShellSurface(QWaylandSurface *surface, QWaylandClient *client, uint id)
 *
 * This signal is emitted when the \a client has requested a shell surface to be associated
 * with \a surface and be assigned the given \a id. The handler for this signal is
 * expected to create the shell surface and initialize it within the scope of the
 * signal emission.
 */

/*!
 * \internal
 */
QByteArray QWaylandWlShell::interfaceName()
{
    return QWaylandWlShellPrivate::interfaceName();
}

/*!
 * \qmltype WlShellSurface
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \brief A wl_shell_surface providing desktop-style compositor-specific features to a surface.
 *
 * This type is part of the \l{WlShell} extension and provides a way to extend
 * the functionality of an existing WaylandSurface with features specific to desktop-style
 * compositors, such as resizing and moving the surface.
 *
 * It corresponds to the Wayland interface wl_shell_surface.
 */

/*!
 * \class QWaylandWlShellSurface
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief A shell surface providing desktop-style compositor-specific features to a surface.
 *
 * This class is part of the QWaylandWlShell extension and provides a way to extend
 * the functionality of an existing QWaylandSurface with features specific to desktop-style
 * compositors, such as resizing and moving the surface.
 *
 * It corresponds to the Wayland interface wl_shell_surface.
 */

/*!
 * Constructs a QWaylandWlShellSurface.
 */
QWaylandWlShellSurface::QWaylandWlShellSurface()
    : QWaylandShellSurfaceTemplate<QWaylandWlShellSurface>(*new QWaylandWlShellSurfacePrivate)
{
}

/*!
 * Constructs a QWaylandWlShellSurface for \a surface and initializes it with the given \a shell and \a resource.
 */
QWaylandWlShellSurface::QWaylandWlShellSurface(QWaylandWlShell *shell, QWaylandSurface *surface, const QWaylandResource &res)
    : QWaylandShellSurfaceTemplate<QWaylandWlShellSurface>(*new QWaylandWlShellSurfacePrivate)
{
    initialize(shell, surface, res);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WlShellSurface::initialize(object shell, object surface, object client, int id)
 *
 * Initializes the WlShellSurface, associating it with the given \a shell, \a surface, \a client, and \a id.
 */

/*!
 * Initializes the QWaylandWlShellSurface, associating it with the given \a shell, \a surface and \a resource.
 */
void QWaylandWlShellSurface::initialize(QWaylandWlShell *shell, QWaylandSurface *surface, const QWaylandResource &resource)
{
    Q_D(QWaylandWlShellSurface);
    d->m_shell = shell;
    d->m_surface = surface;
    d->init(resource.resource());
    setExtensionContainer(surface);
    emit surfaceChanged();
    QWaylandCompositorExtension::initialize();
}

/*!
 * \internal
 */
void QWaylandWlShellSurface::initialize()
{
    QWaylandCompositorExtension::initialize();
}

const struct wl_interface *QWaylandWlShellSurface::interface()
{
    return QWaylandWlShellSurfacePrivate::interface();
}

/*!
 * \internal
 */
QByteArray QWaylandWlShellSurface::interfaceName()
{
    return QWaylandWlShellSurfacePrivate::interfaceName();
}

QSize QWaylandWlShellSurface::sizeForResize(const QSizeF &size, const QPointF &delta, QWaylandWlShellSurface::ResizeEdge edge)
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
 * \enum QWaylandWlShellSurface::ResizeEdge
 *
 * This enum type provides a way to specify a specific edge or corner of
 * the surface.
 *
 * \value None No edge.
 * \value TopEdge The top edge.
 * \value BottomEdge The bottom edge.
 * \value LeftEdge The left edge.
 * \value TopLeftEdge The top left corner.
 * \value BottomLeftEdge The bottom left corner.
 * \value RightEdge The right edge.
 * \value TopRightEdge The top right corner.
 * \value BottomRightEdge The bottom right corner.
 */

/*!
 * \qmlmethod void QtWaylandCompositor::WlShellSurface::sendConfigure(size size, enum edges)
 *
 * Sends a configure event to the client, suggesting that it resize its surface to
 * the provided \a size. The \a edges provide a hint about how the surface
 * was resized.
 */

/*!
 * Sends a configure event to the client, suggesting that it resize its surface to
 * the provided \a size. The \a edges provide a hint about how the surface
 * was resized.
 */
void QWaylandWlShellSurface::sendConfigure(const QSize &size, ResizeEdge edges)
{
    Q_D(QWaylandWlShellSurface);
    d->send_configure(edges, size.width(), size.height());
}

/*!
 * \qmlmethod void QtWaylandCompositor::WlShellSurface::sendPopupDone()
 *
 * Sends a popup_done event to the client to indicate that the user has clicked
 * somewhere outside the client's surfaces.
 */

/*!
 * Sends a popup_done event to the client to indicate that the user has clicked
 * somewhere outside the client's surfaces.
 */
void QWaylandWlShellSurface::sendPopupDone()
{
    Q_D(QWaylandWlShellSurface);
    d->send_popup_done();
}

QWaylandQuickShellIntegration *QWaylandWlShellSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    return new QtWayland::WlShellIntegration(item);
}

/*!
 * \qmlproperty object QtWaylandCompositor::WlShellSurface::surface
 *
 * This property holds the wl_surface associated with this WlShellSurface.
 */

/*!
 * \property QWaylandWlShellSurface::surface
 *
 * This property holds the surface associated with this QWaylandWlShellSurface.
 */
QWaylandSurface *QWaylandWlShellSurface::surface() const
{
    Q_D(const QWaylandWlShellSurface);
    return d->m_surface;
}

/*!
 * \enum QWaylandWlShellSurface::FocusPolicy
 *
 * This enum type is used to specify the focus policy of a shell surface.
 *
 * \value DefaultFocus The default focus policy should be used.
 * \value NoKeyboardFocus The shell surface should not get keyboard focus.
 */

/*!
 * \qmlproperty enum QtWaylandCompositor::WlShellSurface::focusPolicy
 *
 * This property holds the focus policy of the WlShellSurface.
 */

/*!
 * \property QWaylandWlShellSurface::focusPolicy
 *
 * This property holds the focus policy of the QWaylandWlShellSurface.
 */
QWaylandWlShellSurface::FocusPolicy QWaylandWlShellSurface::focusPolicy() const
{
    Q_D(const QWaylandWlShellSurface);
    return d->m_focusPolicy;
}

/*!
 * \qmlproperty string QtWaylandCompositor::WlShellSurface::title
 *
 * This property holds the title of the WlShellSurface.
 */

/*!
 * \property QWaylandWlShellSurface::title
 *
 * This property holds the title of the QWaylandWlShellSurface.
 */
QString QWaylandWlShellSurface::title() const
{
    Q_D(const QWaylandWlShellSurface);
    return d->m_title;
}

/*!
 * \qmlproperty string QtWaylandCompositor::WlShellSurface::className
 *
 * This property holds the class name of the WlShellSurface.
 */

/*!
 * \property QWaylandWlShellSurface::className
 *
 * This property holds the class name of the QWaylandWlShellSurface.
 */
QString QWaylandWlShellSurface::className() const
{
    Q_D(const QWaylandWlShellSurface);
    return d->m_className;
}

QWaylandSurfaceRole *QWaylandWlShellSurface::role()
{
    return &QWaylandWlShellSurfacePrivate::s_role;
}

/*!
 * \qmlmethod void QtWaylandCompositor::WlShellSurface::ping()
 *
 * Sends a ping event to the client. If the client replies to the event the \a pong
 * signal will be emitted.
 */

/*!
 * Sends a ping event to the client. If the client replies to the event the \a pong
 * signal will be emitted.
 */
void QWaylandWlShellSurface::ping()
{
    Q_D(QWaylandWlShellSurface);
    uint32_t serial = d->m_surface->compositor()->nextSerial();
    d->ping(serial);
}

/*!
 * Returns the QWaylandWlShellSurface object associated with the given \a resource, or null if no such object exists.
 */
QWaylandWlShellSurface *QWaylandWlShellSurface::fromResource(wl_resource *resource)
{
    QWaylandWlShellSurfacePrivate::Resource *res = QWaylandWlShellSurfacePrivate::Resource::fromResource(resource);
    if (res)
        return static_cast<QWaylandWlShellSurfacePrivate *>(res->shell_surface_object)->q_func();
    return 0;
}

QT_END_NAMESPACE
