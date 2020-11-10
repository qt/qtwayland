/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Eurogiciel, author: <philippe.coval@eurogiciel.fr>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandxdgshell_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>

#include <QtGui/private/qwindow_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgSurface::Toplevel::Toplevel(QWaylandXdgSurface *xdgSurface)
    : QtWayland::xdg_toplevel(xdgSurface->get_toplevel())
    , m_xdgSurface(xdgSurface)
{
    QWindow *window = xdgSurface->window()->window();
    if (auto *decorationManager = m_xdgSurface->m_shell->decorationManager()) {
        if (!(window->flags() & Qt::FramelessWindowHint))
            m_decoration = decorationManager->createToplevelDecoration(object());
    }
    requestWindowStates(window->windowStates());
    requestWindowFlags(window->flags());
}

QWaylandXdgSurface::Toplevel::~Toplevel()
{
    // The protocol spec requires that the decoration object is deleted before xdg_toplevel.
    delete m_decoration;
    m_decoration = nullptr;

    if (isInitialized())
        destroy();
}

void QWaylandXdgSurface::Toplevel::applyConfigure()
{
    if (!(m_applied.states & (Qt::WindowMaximized|Qt::WindowFullScreen)))
        m_normalSize = m_xdgSurface->m_window->windowFrameGeometry().size();

    if ((m_pending.states & Qt::WindowActive) && !(m_applied.states & Qt::WindowActive)
        && !m_xdgSurface->m_window->display()->isKeyboardAvailable())
        m_xdgSurface->m_window->display()->handleWindowActivated(m_xdgSurface->m_window);

    if (!(m_pending.states & Qt::WindowActive) && (m_applied.states & Qt::WindowActive)
        && !m_xdgSurface->m_window->display()->isKeyboardAvailable())
        m_xdgSurface->m_window->display()->handleWindowDeactivated(m_xdgSurface->m_window);

    m_xdgSurface->m_window->handleToplevelWindowTilingStatesChanged(m_toplevelStates);
    m_xdgSurface->m_window->handleWindowStatesChanged(m_pending.states);

    if (m_pending.size.isEmpty()) {
        // An empty size in the configure means it's up to the client to choose the size
        bool normalPending = !(m_pending.states & (Qt::WindowMaximized|Qt::WindowFullScreen));
        if (normalPending && !m_normalSize.isEmpty())
            m_xdgSurface->m_window->resizeFromApplyConfigure(m_normalSize);
    } else {
        m_xdgSurface->m_window->resizeFromApplyConfigure(m_pending.size);
    }

    m_applied = m_pending;
    qCDebug(lcQpaWayland) << "Applied pending xdg_toplevel configure event:" << m_applied.size << m_applied.states;
}

bool QWaylandXdgSurface::Toplevel::wantsDecorations()
{
    if (m_decoration && (m_decoration->pending() == QWaylandXdgToplevelDecorationV1::mode_server_side
                         || !m_decoration->isConfigured()))
        return false;

    return !(m_pending.states & Qt::WindowFullScreen);
}

void QWaylandXdgSurface::Toplevel::xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states)
{
    m_pending.size = QSize(width, height);

    auto *xdgStates = static_cast<uint32_t *>(states->data);
    size_t numStates = states->size / sizeof(uint32_t);

    m_pending.states = Qt::WindowNoState;
    m_toplevelStates = QWaylandWindow::WindowNoState;

    for (size_t i = 0; i < numStates; i++) {
        switch (xdgStates[i]) {
        case XDG_TOPLEVEL_STATE_ACTIVATED:
            m_pending.states |= Qt::WindowActive;
            break;
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            m_pending.states |= Qt::WindowMaximized;
            break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            m_pending.states |= Qt::WindowFullScreen;
            break;
        case XDG_TOPLEVEL_STATE_TILED_LEFT:
            m_toplevelStates |= QWaylandWindow::WindowTiledLeft;
            break;
        case XDG_TOPLEVEL_STATE_TILED_RIGHT:
            m_toplevelStates |= QWaylandWindow::WindowTiledRight;
            break;
        case XDG_TOPLEVEL_STATE_TILED_TOP:
            m_toplevelStates |= QWaylandWindow::WindowTiledTop;
            break;
        case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
            m_toplevelStates |= QWaylandWindow::WindowTiledBottom;
            break;
        default:
            break;
        }
    }
    qCDebug(lcQpaWayland) << "Received xdg_toplevel.configure with" << m_pending.size
                          << "and" << m_pending.states;
}

void QWaylandXdgSurface::Toplevel::xdg_toplevel_close()
{
    m_xdgSurface->m_window->window()->close();
}

void QWaylandXdgSurface::Toplevel::requestWindowFlags(Qt::WindowFlags flags)
{
    if (m_decoration) {
        if (flags & Qt::FramelessWindowHint) {
            delete m_decoration;
            m_decoration = nullptr;
        } else {
            m_decoration->unsetMode();
        }
    }
}

void QWaylandXdgSurface::Toplevel::requestWindowStates(Qt::WindowStates states)
{
    // Re-send what's different from the applied state
    Qt::WindowStates changedStates = m_applied.states ^ states;

    if (changedStates & Qt::WindowMaximized) {
        if (states & Qt::WindowMaximized)
            set_maximized();
        else
            unset_maximized();
    }

    if (changedStates & Qt::WindowFullScreen) {
        if (states & Qt::WindowFullScreen) {
            auto screen = m_xdgSurface->window()->waylandScreen();
            if (screen) {
                set_fullscreen(screen->output());
            }
        } else
            unset_fullscreen();
    }

    // Minimized state is not reported by the protocol, so always send it
    if (states & Qt::WindowMinimized) {
        set_minimized();
        m_xdgSurface->window()->handleWindowStatesChanged(states & ~Qt::WindowMinimized);
    }
}

QtWayland::xdg_toplevel::resize_edge QWaylandXdgSurface::Toplevel::convertToResizeEdges(Qt::Edges edges)
{
    return static_cast<enum resize_edge>(
                ((edges & Qt::TopEdge) ? resize_edge_top : 0)
                | ((edges & Qt::BottomEdge) ? resize_edge_bottom : 0)
                | ((edges & Qt::LeftEdge) ? resize_edge_left : 0)
                | ((edges & Qt::RightEdge) ? resize_edge_right : 0));
}

QWaylandXdgSurface::Popup::Popup(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parent,
                                 QtWayland::xdg_positioner *positioner)
    : xdg_popup(xdgSurface->get_popup(parent->object(), positioner->object()))
    , m_xdgSurface(xdgSurface)
    , m_parent(parent)
{
}

QWaylandXdgSurface::Popup::~Popup()
{
    if (isInitialized())
        destroy();

    if (m_grabbing) {
        auto *shell = m_xdgSurface->m_shell;
        Q_ASSERT(shell->m_topmostGrabbingPopup == this);
        shell->m_topmostGrabbingPopup = m_parent->m_popup;
    }
}

void QWaylandXdgSurface::Popup::grab(QWaylandInputDevice *seat, uint serial)
{
    m_xdgSurface->m_shell->m_topmostGrabbingPopup = this;
    xdg_popup::grab(seat->wl_seat(), serial);
    m_grabbing = true;
}

void QWaylandXdgSurface::Popup::xdg_popup_popup_done()
{
    m_xdgSurface->m_window->window()->close();
}

QWaylandXdgSurface::QWaylandXdgSurface(QWaylandXdgShell *shell, ::xdg_surface *surface, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , xdg_surface(surface)
    , m_shell(shell)
    , m_window(window)
{
    QWaylandDisplay *display = window->display();
    Qt::WindowType type = window->window()->type();
    auto *transientParent = window->transientParent();

    if (type == Qt::ToolTip && transientParent) {
        setPopup(transientParent);
    } else if (type == Qt::Popup && transientParent && display->lastInputDevice()) {
        setGrabPopup(transientParent, display->lastInputDevice(), display->lastInputSerial());
    } else {
        setToplevel();
        if (transientParent) {
            auto parentXdgSurface = static_cast<QWaylandXdgSurface *>(transientParent->shellSurface());
            if (parentXdgSurface)
                m_toplevel->set_parent(parentXdgSurface->m_toplevel->object());
        }
    }
    setSizeHints();
}

QWaylandXdgSurface::~QWaylandXdgSurface()
{
    if (m_toplevel) {
        delete m_toplevel;
        m_toplevel = nullptr;
    }
    if (m_popup) {
        delete m_popup;
        m_popup = nullptr;
    }
    destroy();
}

bool QWaylandXdgSurface::resize(QWaylandInputDevice *inputDevice, Qt::Edges edges)
{
    if (!m_toplevel || !m_toplevel->isInitialized())
        return false;

    auto resizeEdges = Toplevel::convertToResizeEdges(edges);
    m_toplevel->resize(inputDevice->wl_seat(), inputDevice->serial(), resizeEdges);
    return true;
}

bool QWaylandXdgSurface::move(QWaylandInputDevice *inputDevice)
{
    if (m_toplevel && m_toplevel->isInitialized()) {
        m_toplevel->move(inputDevice->wl_seat(), inputDevice->serial());
        return true;
    }
    return false;
}

bool QWaylandXdgSurface::showWindowMenu(QWaylandInputDevice *seat)
{
    if (m_toplevel && m_toplevel->isInitialized()) {
        QPoint position = seat->pointerSurfacePosition().toPoint();
        m_toplevel->show_window_menu(seat->wl_seat(), seat->serial(), position.x(), position.y());
        return true;
    }
    return false;
}

void QWaylandXdgSurface::setTitle(const QString &title)
{
    if (m_toplevel)
        m_toplevel->set_title(title);
}

void QWaylandXdgSurface::setAppId(const QString &appId)
{
    if (m_toplevel)
        m_toplevel->set_app_id(appId);

    m_appId = appId;
}

void QWaylandXdgSurface::setWindowFlags(Qt::WindowFlags flags)
{
    if (m_toplevel)
        m_toplevel->requestWindowFlags(flags);
}

bool QWaylandXdgSurface::isExposed() const
{
    return m_configured || m_pendingConfigureSerial;
}

bool QWaylandXdgSurface::handleExpose(const QRegion &region)
{
    if (!isExposed() && !region.isEmpty()) {
        m_exposeRegion = region;
        return true;
    }
    return false;
}

void QWaylandXdgSurface::applyConfigure()
{
    // It is a redundant ack_configure, so skipped.
    if (m_pendingConfigureSerial == m_appliedConfigureSerial)
        return;

    if (m_toplevel)
        m_toplevel->applyConfigure();
    m_appliedConfigureSerial = m_pendingConfigureSerial;

    m_configured = true;
    ack_configure(m_appliedConfigureSerial);
}

bool QWaylandXdgSurface::wantsDecorations() const
{
    return m_toplevel && m_toplevel->wantsDecorations();
}

void QWaylandXdgSurface::propagateSizeHints()
{
    setSizeHints();

    if (m_toplevel && m_window)
        m_window->commit();
}

void QWaylandXdgSurface::setWindowGeometry(const QRect &rect)
{
    set_window_geometry(rect.x(), rect.y(), rect.width(), rect.height());
}

void QWaylandXdgSurface::setSizeHints()
{
    if (m_toplevel && m_window) {
        const int minWidth = qMax(0, m_window->windowMinimumSize().width());
        const int minHeight = qMax(0, m_window->windowMinimumSize().height());
        m_toplevel->set_min_size(minWidth, minHeight);

        int maxWidth = qMax(0, m_window->windowMaximumSize().width());
        if (maxWidth == QWINDOWSIZE_MAX)
            maxWidth = 0;
        int maxHeight = qMax(0, m_window->windowMaximumSize().height());
        if (maxHeight == QWINDOWSIZE_MAX)
            maxHeight = 0;
        m_toplevel->set_max_size(maxWidth, maxHeight);
    }
}

void *QWaylandXdgSurface::nativeResource(const QByteArray &resource)
{
    QByteArray lowerCaseResource = resource.toLower();
    if (lowerCaseResource == "xdg_surface")
        return object();
    else if (lowerCaseResource == "xdg_toplevel" && m_toplevel)
        return m_toplevel->object();
    else if (lowerCaseResource == "xdg_popup" && m_popup)
        return m_popup->object();
    return nullptr;
}

void QWaylandXdgSurface::requestWindowStates(Qt::WindowStates states)
{
    if (m_toplevel)
        m_toplevel->requestWindowStates(states);
    else
        qCDebug(lcQpaWayland) << "Ignoring window states requested by non-toplevel zxdg_surface_v6.";
}

void QWaylandXdgSurface::setToplevel()
{
    Q_ASSERT(!m_toplevel && !m_popup);
    m_toplevel = new Toplevel(this);
}

void QWaylandXdgSurface::setPopup(QWaylandWindow *parent)
{
    Q_ASSERT(!m_toplevel && !m_popup);

    auto parentXdgSurface = static_cast<QWaylandXdgSurface *>(parent->shellSurface());

    auto positioner = new QtWayland::xdg_positioner(m_shell->create_positioner());
    // set_popup expects a position relative to the parent
    QPoint transientPos = m_window->geometry().topLeft(); // this is absolute
    transientPos -= parent->geometry().topLeft();
    if (parent->decoration()) {
        transientPos.setX(transientPos.x() + parent->decoration()->margins().left());
        transientPos.setY(transientPos.y() + parent->decoration()->margins().top());
    }
    positioner->set_anchor_rect(transientPos.x(), transientPos.y(), 1, 1);
    positioner->set_anchor(QtWayland::xdg_positioner::anchor_top_left);
    positioner->set_gravity(QtWayland::xdg_positioner::gravity_bottom_right);
    positioner->set_size(m_window->geometry().width(), m_window->geometry().height());
    m_popup = new Popup(this, parentXdgSurface, positioner);
    positioner->destroy();
    delete positioner;
}

void QWaylandXdgSurface::setGrabPopup(QWaylandWindow *parent, QWaylandInputDevice *device, int serial)
{
    auto parentXdgSurface = static_cast<QWaylandXdgSurface *>(parent->shellSurface());
    auto *top = m_shell->m_topmostGrabbingPopup;

    if (top && top->m_xdgSurface != parentXdgSurface) {
        qCWarning(lcQpaWayland) << "setGrabPopup called with a parent," << parentXdgSurface
                                << "which does not match the current topmost grabbing popup,"
                                << top->m_xdgSurface << "According to the xdg-shell protocol, this"
                                << "is not allowed. The wayland QPA plugin is currently handling"
                                << "it by setting the parent to the topmost grabbing popup."
                                << "Note, however, that this may cause positioning errors and"
                                << "popups closing unxpectedly because xdg-shell mandate that child"
                                << "popups close before parents";
        parent = top->m_xdgSurface->m_window;
    }
    setPopup(parent);
    m_popup->grab(device, serial);
}

void QWaylandXdgSurface::xdg_surface_configure(uint32_t serial)
{
    m_pendingConfigureSerial = serial;
    if (!m_configured) {
        // We have to do the initial applyConfigure() immediately, since that is the expose.
        applyConfigure();
        m_exposeRegion = QRegion(QRect(QPoint(), m_window->geometry().size()));
    } else {
        // Later configures are probably resizes, so we have to queue them up for a time when we
        // are not painting to the window.
        m_window->applyConfigureWhenPossible();
    }

    if (!m_exposeRegion.isEmpty()) {
        m_window->handleExpose(m_exposeRegion);
        m_exposeRegion = QRegion();
    }
}

bool QWaylandXdgSurface::requestActivate()
{
    if (auto *activation = m_shell->activation()) {
        activation->activate(m_activationToken, window()->wlSurface());
        return true;
    }
    return false;
}

void QWaylandXdgSurface::requestXdgActivationToken(quint32 serial)
{
    if (auto *activation = m_shell->activation()) {
        auto tokenProvider = activation->requestXdgActivationToken(
                m_shell->m_display, m_window->wlSurface(), serial, m_appId);
        connect(tokenProvider, &QWaylandXdgActivationTokenV1::done, this,
                [this, tokenProvider](const QString &token) {
                    Q_EMIT m_window->xdgActivationTokenCreated(token);
                    tokenProvider->deleteLater();
                });
    } else {
        QWaylandShellSurface::requestXdgActivationToken(serial);
    }
}

void QWaylandXdgSurface::setXdgActivationToken(const QString &token)
{
    if (auto *activation = m_shell->activation()) {
        m_activationToken = token;
    } else {
        qCWarning(lcQpaWayland) << "zxdg_activation_v1 not available";
    }
}

QWaylandXdgShell::QWaylandXdgShell(QWaylandDisplay *display, uint32_t id, uint32_t availableVersion)
    : QtWayland::xdg_wm_base(display->wl_registry(), id, qMin(availableVersion, 2u))
    , m_display(display)
{
    display->addRegistryListener(&QWaylandXdgShell::handleRegistryGlobal, this);
}

QWaylandXdgShell::~QWaylandXdgShell()
{
    m_display->removeListener(&QWaylandXdgShell::handleRegistryGlobal, this);
    destroy();
}

QWaylandXdgSurface *QWaylandXdgShell::getXdgSurface(QWaylandWindow *window)
{
    return new QWaylandXdgSurface(this, get_xdg_surface(window->wlSurface()), window);
}

void QWaylandXdgShell::xdg_wm_base_ping(uint32_t serial)
{
    pong(serial);
}

void QWaylandXdgShell::handleRegistryGlobal(void *data, wl_registry *registry, uint id,
                                            const QString &interface, uint version)
{
    QWaylandXdgShell *xdgShell = static_cast<QWaylandXdgShell *>(data);
    if (interface == QLatin1String(QWaylandXdgDecorationManagerV1::interface()->name))
        xdgShell->m_xdgDecorationManager.reset(new QWaylandXdgDecorationManagerV1(registry, id, version));

    if (interface == QLatin1String(QWaylandXdgActivationV1::interface()->name)) {
        xdgShell->m_xdgActivation.reset(new QWaylandXdgActivationV1(registry, id, version));
    }
}

}

QT_END_NAMESPACE
