// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandqtshell.h"
#include "qwaylandqtshell_p.h"
#include "qwaylandqtshellchrome.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include "qwaylandqtshell.h"
#include <QtWaylandCompositor/QWaylandResource>

#if QT_CONFIG(wayland_compositor_quick)
# include "qwaylandqtshellintegration_p.h"
#endif

#include <QtWaylandCompositor/QWaylandResource>
#include <QDebug>
#include <compositor/compositor_api/qwaylandseat.h>

#include <QtWaylandCompositor/private/qwaylandutils_p.h>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype QtShell
 * \instantiates QWaylandQtShell
 * \inqmlmodule QtWayland.Compositor.QtShell
 * \since 6.3
 * \brief Provides a shell extension for Qt applications running on a Qt Wayland Compositor.
 *
 * The QtShell extension provides a way to associate an QtShellSurface with a regular Wayland
 * surface. The QtShell extension is written to support the window management features which are
 * supported by Qt. It may be suitable on a platform where both the compositor and client
 * applications are written with Qt, and where applications are trusted not to abuse features such
 * as manual window positioning and "bring-to-front".
 *
 * For other use cases, consider using IviApplication or XdgShell instead.
 *
 * \qml
 * import QtWayland.Compositor.QtShell
 *
 * WaylandCompositor {
 *     property ListModel shellSurfaces: ListModel {}
 *     QtShell {
 *         onQtShellSurfaceCreated: {
 *              shellSurfaces.append({shellSurface: qtShellSurface})
 *         }
 *     }
 * }
 * \endqml
 */
QWaylandQtShell::QWaylandQtShell()
    : QWaylandCompositorExtensionTemplate<QWaylandQtShell>(*new QWaylandQtShellPrivate())
{
}

QWaylandQtShell::QWaylandQtShell(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandQtShell>(compositor, *new QWaylandQtShellPrivate())
{
}

bool QWaylandQtShell::moveChromeToFront(QWaylandQtShellChrome *chrome)
{
    Q_D(QWaylandQtShell);
    for (int i = 0; i < d->m_chromes.size(); ++i) {
        if (d->m_chromes.at(i) == chrome) {
            if (i > 0) {
                QWaylandQtShellChrome *currentActive = d->m_chromes.first();
                d->m_chromes.move(i, 0);
                chrome->activate();
                currentActive->deactivate();
            }
            return true;
        }
    }

    return false;
}

void QWaylandQtShell::registerChrome(QWaylandQtShellChrome *chrome)
{
    Q_D(QWaylandQtShell);
    if (moveChromeToFront(chrome))
        return;

    QWaylandQtShellChrome *currentActive = d->m_chromes.isEmpty() ? nullptr : d->m_chromes.first();

    d->m_chromes.prepend(chrome);
    chrome->activate();

    if (currentActive != nullptr)
        currentActive->deactivate();

    connect(chrome, &QWaylandQtShellChrome::activated, this, &QWaylandQtShell::chromeActivated);
    connect(chrome, &QWaylandQtShellChrome::deactivated, this, &QWaylandQtShell::chromeDeactivated);
}

void QWaylandQtShell::unregisterChrome(QWaylandQtShellChrome *chrome)
{
    Q_D(QWaylandQtShell);

    chrome->disconnect(this);
    int index = d->m_chromes.indexOf(chrome);
    if (index >= 0) {
        d->m_chromes.removeAt(index);
        if (index == 0 && d->m_chromes.size() > 0)
            d->m_chromes.at(0)->activate();
    }
}

void QWaylandQtShell::chromeActivated()
{
    QWaylandQtShellChrome *c = qobject_cast<QWaylandQtShellChrome *>(sender());
    if (c != nullptr) {
        moveChromeToFront(c);
    }
}

void QWaylandQtShell::chromeDeactivated()
{
    Q_D(QWaylandQtShell);
    QWaylandQtShellChrome *c = qobject_cast<QWaylandQtShellChrome *>(sender());
    if (d->m_chromes.size() > 1 && d->m_chromes.at(0) == c) {
        d->m_chromes.move(0, 1);
        d->m_chromes.at(0)->activate();
    } else if (d->m_chromes.size() == 1) { // One window must be active
        d->m_chromes.at(0)->activate();
    }
}

void QWaylandQtShell::initialize()
{
    Q_D(QWaylandQtShell);
    QWaylandCompositorExtensionTemplate::initialize();

    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandQtShell";
        return;
    }

    d->init(compositor->display(), 1);
}

const struct wl_interface *QWaylandQtShell::interface()
{
    return QWaylandQtShellPrivate::interface();
}

/*!
 * \internal
 */
QByteArray QWaylandQtShell::interfaceName()
{
    return QWaylandQtShellPrivate::interfaceName();
}

/*!
 * \qmlsignal void QtShell::qtShellSurfaceRequested(WaylandSurface surface, WaylandResource resource)
 *
 * This signal is emitted when the client has requested a QtShellSurface to be associated
 * with \a surface. The handler for this signal is expected to create the QtShellSurface for
 * \a resource and initialize it within the scope of the signal emission. If no QtShellSurface is
 * created, a default one will be created instead.
 */

/*!
 * \qmlsignal void QtShell::qtShellSurfaceCreated(QtShellSurface *qtShellSurface)
 *
 * This signal is emitted when an QtShellSurface has been created. The supplied \a qtShellSurface is
 * most commonly used to instantiate a ShellSurfaceItem.
 */

QWaylandQtShellPrivate::QWaylandQtShellPrivate()
{
}

void QWaylandQtShellPrivate::unregisterQtShellSurface(QWaylandQtShellSurface *qtShellSurface)
{
    Q_UNUSED(qtShellSurface)
}

void QWaylandQtShellPrivate::zqt_shell_v1_surface_create(QtWaylandServer::zqt_shell_v1::Resource *resource, wl_resource *surfaceResource, uint32_t id)
{
    Q_Q(QWaylandQtShell);
    QWaylandSurface *surface = QWaylandSurface::fromResource(surfaceResource);

    if (!surface->setRole(QWaylandQtShellSurface::role(), resource->handle, ZQT_SHELL_V1_ERROR_ROLE))
        return;

    QWaylandResource qtShellSurfaceResource(wl_resource_create(resource->client(), &zqt_shell_surface_v1_interface,
                                                           wl_resource_get_version(resource->handle), id));

    emit q->qtShellSurfaceRequested(surface, qtShellSurfaceResource);

    QWaylandQtShellSurface *qtShellSurface = QWaylandQtShellSurface::fromResource(qtShellSurfaceResource.resource());

    if (!qtShellSurface)
        qtShellSurface = new QWaylandQtShellSurface(q, surface, qtShellSurfaceResource);

    emit q->qtShellSurfaceCreated(qtShellSurface);
}

QWaylandSurfaceRole QWaylandQtShellSurfacePrivate::s_role("qt_shell_surface");

/*!
 * \qmltype QtShellSurface
 * \instantiates QWaylandQtShellSurface
 * \inqmlmodule QtWayland.Compositor.QtShell
 * \since 6.3
 * \brief Provides a simple way to identify and resize a surface.
 *
 * This type is part of the \l{QtShell} extension and provides a way to extend
 * the functionality of an existing WaylandSurface with window management functionality.
 *
 * The QtShellSurface type holds the core functionality needed to create a compositor that supports
 * the QtShell extension. It can be used directly, or via the QtShellChrome type, depending on what
 * the needs of the compositor are. The QtShellChrome type has default behaviors and convenience
 * APIs for working with QtShellSurface objects.
 */

/*!
  \qmlsignal void QtShellSurface::startMove()

  The client has requested an interactive move operation in the compositor by calling
  \l{QWindow::startSystemMove()}.

  \sa capabilities
*/

/*!
  \qmlsignal void QtShellSurface::startResize(enum edges)

  The client has requested an interactive resize operation in the compositor by calling
  \l{QWindow::startSystemResize()}.

  The \a edges provides information about which edge of the window should be moved during the
  resize. It is a mask of the following values:
  \list
    \li Qt.TopEdge
    \li Qt.LeftEdge
    \li Qt.RightEdge
    \li Qt.BottomEdge
 \endlist

  \sa capabilities
*/

QWaylandQtShellSurface::QWaylandQtShellSurface()
    : QWaylandShellSurfaceTemplate<QWaylandQtShellSurface>(*new QWaylandQtShellSurfacePrivate())
{
}

QWaylandQtShellSurface::QWaylandQtShellSurface(QWaylandQtShell *application, QWaylandSurface *surface, const QWaylandResource &resource)
    : QWaylandShellSurfaceTemplate<QWaylandQtShellSurface>(*new QWaylandQtShellSurfacePrivate())
{
    initialize(application, surface, resource);
}

/*!
 * \qmlmethod void QtShellSurface::initialize(QtShell qtShell, WaylandSurface surface, WaylandResource resource)
 *
 * Initializes the QtShellSurface, associating it with the given \a qtShell, \a surface, and
 * \a resource.
 */
void QWaylandQtShellSurface::initialize(QWaylandQtShell *qtShell, QWaylandSurface *surface, const QWaylandResource &resource)
{
    Q_D(QWaylandQtShellSurface);

    d->m_qtShell = qtShell;
    d->m_surface = surface;

    connect(d->m_surface, &QWaylandSurface::damaged, this, &QWaylandQtShellSurface::surfaceCommitted);

    d->init(resource.resource());
    setExtensionContainer(surface);

    emit surfaceChanged();

    QWaylandCompositorExtension::initialize();
}

/*!
 * \qmlproperty WaylandSurface QtShellSurface::surface
 *
 * This property holds the surface associated with this QtShellSurface.
 */
QWaylandSurface *QWaylandQtShellSurface::surface() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_surface;
}

QWaylandQtShell *QWaylandQtShellSurface::shell() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_qtShell;
}

/*!
 * \qmlproperty point QtShellSurface::windowPosition
 *
 * This property holds the position of the shell surface relative to its output.
 */
QPoint QWaylandQtShellSurface::windowPosition() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_windowGeometry.topLeft();
}

void QWaylandQtShellSurface::setWindowPosition(const QPoint &position)
{
    Q_D(QWaylandQtShellSurface);

    // We don't care about the ack in this case, so use UINT_MAX as serial
    d->send_set_position(UINT32_MAX, position.x(), position.y());
    d->send_configure(UINT32_MAX);

    d->m_windowGeometry.moveTopLeft(position);
    d->m_positionSet = true;
    emit positionAutomaticChanged();
    emit windowGeometryChanged();
}

/*!
 * \qmlproperty rect QtShellSurface::windowGeometry
 *
 * This property holds the window geometry of the shell surface.
 */
QRect QWaylandQtShellSurface::windowGeometry() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_windowGeometry;
}

/*!
 * \qmlproperty size QtShellSurface::minimumSize
 *
 * The minimum size of the window if the client has specified one. Otherwise an invalid size.
 */
QSize QWaylandQtShellSurface::minimumSize() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_minimumSize;
}

/*!
 * \qmlproperty size QtShellSurface::maximumSize
 *
 * The maximum size of the window if the client has specified one. Otherwise an invalid size.
 */
QSize QWaylandQtShellSurface::maximumSize() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_maximumSize;
}

/*!
 *  \qmlmethod void QtShellSurface::requestWindowGeometry(int windowState, rect windowGeometry)
 *
 *  Requests a new \a windowState and \a windowGeometry for the QtShellSurface. The state and
 *  geometry is updated when the client has acknowledged the request (at which point it is safe to
 *  assume that the surface's buffer has been resized if necessary).
 */
void QWaylandQtShellSurface::requestWindowGeometry(uint windowState, const QRect &windowGeometry)
{
    Q_D(QWaylandQtShellSurface);
    if (!windowGeometry.isValid())
        return;

    d->configure(windowState, windowGeometry);
}

void QWaylandQtShellSurfacePrivate::configure(uint windowState, const QRect &newGeometry)
{
    QWaylandCompositor *compositor = m_surface != nullptr ? m_surface->compositor() : nullptr;
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when configuring QWaylandQtShell";
        return;
    }

    uint32_t serial = compositor->nextSerial();
    m_pendingConfigures[serial] = qMakePair(windowState, newGeometry);

    send_set_position(serial, newGeometry.x(), newGeometry.y());
    send_resize(serial, newGeometry.width(), newGeometry.height());
    send_set_window_state(serial, windowState & ~Qt::WindowActive);
    send_configure(serial);
}

void QWaylandQtShellSurface::setFrameMargins(const QMargins &margins)
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_frameMargins == margins)
        return;

    d->m_frameMargins = margins;
    d->updateFrameMargins();

    emit frameMarginChanged();
}

/*!
 *  \qmlproperty int QtShellSurface::frameMarginLeft
 *
 *  This holds the window frame margin to the left of the surface.
 */
void QWaylandQtShellSurface::setFrameMarginLeft(int left)
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_frameMargins.left() == left)
        return;

    d->m_frameMargins.setLeft(left);
    d->updateFrameMargins();

    emit frameMarginChanged();
}

int QWaylandQtShellSurface::frameMarginLeft() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_frameMargins.left();
}

/*!
 *  \qmlproperty int QtShellSurface::frameMarginRight
 *
 *  This holds the window frame margin to the right of the surface.
 */
void QWaylandQtShellSurface::setFrameMarginRight(int right)
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_frameMargins.right() == right)
        return;

    d->m_frameMargins.setRight(right);
    d->updateFrameMargins();

    emit frameMarginChanged();
}

int QWaylandQtShellSurface::frameMarginRight() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_frameMargins.right();
}

/*!
 *  \qmlproperty int QtShellSurface::frameMarginTop
 *
 *  This holds the window frame margin above the surface.
 */

void QWaylandQtShellSurface::setFrameMarginTop(int top)
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_frameMargins.top() == top)
        return;
    d->m_frameMargins.setTop(top);
    d->updateFrameMargins();

    emit frameMarginChanged();
}

int QWaylandQtShellSurface::frameMarginTop() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_frameMargins.top();
}

/*!
 *  \qmlproperty int QtShellSurface::frameMarginBottom
 *
 *  This holds the window frame margin below the surface.
 */
void QWaylandQtShellSurface::setFrameMarginBottom(int bottom)
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_frameMargins.bottom() == bottom)
        return;
    d->m_frameMargins.setBottom(bottom);
    d->updateFrameMargins();

    emit frameMarginChanged();
}

bool QWaylandQtShellSurface::positionAutomatic() const
{
    Q_D(const QWaylandQtShellSurface);
    return !d->m_positionSet;
}

int QWaylandQtShellSurface::frameMarginBottom() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_frameMargins.bottom();
}

/*!
 *  \qmlproperty int QtShellSurface::windowFlags
 *
 *  This property holds the window flags of the QtShellSurface.
 */
uint QWaylandQtShellSurface::windowFlags() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_windowFlags;
}

/*!
 *  \qmlmethod void QtShellSurface::sendClose()
 *
 *  Requests that the client application closes itself.
 */
void QWaylandQtShellSurface::sendClose()
{
    Q_D(QWaylandQtShellSurface);
    d->send_close();
}

/*!
 *  \qmlproperty string QtShellSurface::windowTitle
 *
 *  This property holds the window title of the QtShellSurface.
 */
QString QWaylandQtShellSurface::windowTitle() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_windowTitle;
}

/*!
 *  \qmlproperty bool QtShellSurface::active
 *
 *  This property holds whether the surface is currently considered active.
 *
 *  \note There are no restrictions in QtShellSurface that prevents multiple surfaces from being
 *  active simultaneously. Such logic must either be implemented by the compositor itself, or by
 *  using the QtShellChrome type, which will automatically manage the activation state of surfaces.
 */
void QWaylandQtShellSurface::setActive(bool active)
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_active == active)
        return;

    d->m_active = active;
    QWaylandCompositor *compositor = d->m_surface ? d->m_surface->compositor() : nullptr;
    QWaylandSeat *seat = compositor ? compositor->defaultSeat() : nullptr;
    if (seat && active)
        seat->setKeyboardFocus(surface());
    emit activeChanged();
}

bool QWaylandQtShellSurface::active() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_active;
}

/*!
 *  \qmlproperty enum QtShellSurface::capabilities
 *
 * This property holds the capabilities of the compositor. By default, no special capabilities are
 * enabled.
 *
 * \list
 *   \li QtShellSurface.InteractiveMove The client can trigger a server-side interactive move
 *       operation using \l{QWindow::startSystemMove()}. The compositor will be notified of this
 *       through the \l{startMove()} signal.
 *   \li QtShellSurface.InteractiveResize The client can trigger a server-side interactive resize
 *       operation using \l{QWindow::startSystemResize()}. The compositor will be notified of this
 *       through the \l{startResize()} signal.
 * \endlist
 */
void QWaylandQtShellSurface::setCapabilities(CapabilityFlags capabilities)
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_capabilities == capabilities)
        return;

    d->m_capabilities = capabilities;
    d->send_set_capabilities(capabilities);

    emit capabilitiesChanged();
}

QWaylandQtShellSurface::CapabilityFlags QWaylandQtShellSurface::capabilities() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_capabilities;
}

/*!
 *  \qmlproperty int QtShellSurface::windowState
 *
 *  This property holds the window state of the QtShellSurface.
 *
 *  \note When \l{requestWindowGeometry()} is called to update state of the surface, the
 *  \c windowState property will not be updated until the client has acknowledged the state change.
 */
uint QWaylandQtShellSurface::windowState() const
{
    Q_D(const QWaylandQtShellSurface);
    return d->m_windowState;
}

void QWaylandQtShellSurface::surfaceCommitted()
{
    Q_D(QWaylandQtShellSurface);
    if (d->m_lastAckedConfigure < UINT32_MAX) {
        QRect targetRect = d->m_windowGeometry;
        uint windowState = d->m_windowState;
        for (auto it = d->m_pendingConfigures.begin(); it != d->m_pendingConfigures.end(); ) {
            if (it.key() == d->m_lastAckedConfigure) {
                targetRect = it.value().second;
                windowState = it.value().first;
            }

            if (it.key() <= d->m_lastAckedConfigure)
                it = d->m_pendingConfigures.erase(it);
            else
                break;
        }

        if (d->m_windowState != windowState) {
            d->m_windowState = windowState;
            emit windowStateChanged();
        }

        if (d->m_windowGeometry != targetRect) {
            d->m_windowGeometry = targetRect;
            d->m_positionSet = true;
            emit positionAutomaticChanged();
            emit windowGeometryChanged();
        }

        d->m_lastAckedConfigure = UINT32_MAX;
        d->m_pendingPosition = QPoint{};
        d->m_pendingPositionValid = false;
        d->m_pendingSize = QSize{};
    } else {
        QRect oldRect = d->m_windowGeometry;
        if (d->m_pendingPositionValid) {
            d->m_windowGeometry.moveTopLeft(d->m_pendingPosition);
            d->m_pendingPosition = QPoint{};
            d->m_pendingPositionValid = false;
            d->m_positionSet = true;
            emit positionAutomaticChanged();
        }

        if (d->m_pendingSize.isValid()) {
            d->m_windowGeometry.setSize(d->m_pendingSize);
            d->m_pendingSize = QSize{};
        }

        if (d->m_windowGeometry != oldRect)
            emit windowGeometryChanged();
    }
}

/*!
 * Returns the Wayland interface for the QWaylandQtShellSurface.
 */
const wl_interface *QWaylandQtShellSurface::interface()
{
    return QWaylandQtShellSurfacePrivate::interface();
}

QByteArray QWaylandQtShellSurface::interfaceName()
{
    return QWaylandQtShellSurfacePrivate::interfaceName();
}

/*!
 * Returns the surface role for the QWaylandQtShellSurface.
 */
QWaylandSurfaceRole *QWaylandQtShellSurface::role()
{
    return &QWaylandQtShellSurfacePrivate::s_role;
}

/*!
 * Returns the QWaylandQtShellSurface corresponding to the \a resource.
 */
QWaylandQtShellSurface *QWaylandQtShellSurface::fromResource(wl_resource *resource)
{
    if (auto p = QtWayland::fromResource<QWaylandQtShellSurfacePrivate *>(resource))
        return p->q_func();
    return nullptr;
}

#if QT_CONFIG(wayland_compositor_quick)
QWaylandQuickShellIntegration *QWaylandQtShellSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    return new QtWayland::QtShellIntegration(item);
}
#endif

/*!
 * \internal
 */
void QWaylandQtShellSurface::initialize()
{
    QWaylandShellSurfaceTemplate::initialize();
}

QWaylandQtShellSurfacePrivate::QWaylandQtShellSurfacePrivate()
{
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_ack_configure(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    if (serial < UINT32_MAX)
        m_lastAckedConfigure = serial;

    // Fake a surface commit because we won't get one as long as the window is unexposed
    if (m_windowState & Qt::WindowMinimized)
        q->surfaceCommitted();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_reposition(Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);

    m_pendingPosition = QPoint(x, y);
    m_pendingPositionValid = true;
    m_lastAckedConfigure = UINT32_MAX;
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_set_size(Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    m_pendingSize = QSize(width, height);
    m_lastAckedConfigure = UINT32_MAX;
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_set_minimum_size(Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    m_minimumSize = QSize{width, height};
    emit q->minimumSizeChanged();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_set_maximum_size(Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    m_maximumSize = QSize{width, height};
    emit q->maximumSizeChanged();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_destroy_resource(QtWaylandServer::zqt_shell_surface_v1::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    QWaylandQtShellPrivate::get(m_qtShell)->unregisterQtShellSurface(q);
    delete q;
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_destroy(QtWaylandServer::zqt_shell_surface_v1::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_set_window_flags(Resource *resource, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    m_windowFlags = flags;
    emit q->windowFlagsChanged();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_change_window_state(Resource *resource, uint32_t state)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    uint oldWindowState = m_windowState;
    m_windowState = state & ~Qt::WindowActive;

    if (oldWindowState != m_windowState)
        emit q->windowStateChanged();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_start_system_resize(Resource *resource, uint32_t serial, uint32_t edge)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandQtShellSurface);
    emit q->startResize(Qt::Edges(edge));
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_start_system_move(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandQtShellSurface);
    emit q->startMove();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_set_window_title(Resource *resource,
                                                                          const QString &title)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    m_windowTitle = title;
    emit q->windowTitleChanged();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_request_activate(Resource *resource)

{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    q->setActive(true);
}

void QWaylandQtShellSurfacePrivate::updateFrameMargins()
{
    send_set_frame_margins(m_frameMargins.left(), m_frameMargins.right(),
                           m_frameMargins.top(), m_frameMargins.bottom());
}


void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_raise(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    emit q->raiseRequested();
}

void QWaylandQtShellSurfacePrivate::zqt_shell_surface_v1_lower(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandQtShellSurface);
    emit q->lowerRequested();
}

QT_END_NAMESPACE

#include "moc_qwaylandqtshell.cpp"
