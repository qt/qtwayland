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
#include <QtWaylandCompositor/QWaylandClient>

#include <QtCore/QObject>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QWaylandShellPrivate::QWaylandShellPrivate()
    : QWaylandExtensionTemplatePrivate()
    , wl_shell()
{
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
    , m_focusPolicy(QWaylandShellSurface::DefaultFocus)
{
}

QWaylandShellSurfacePrivate::~QWaylandShellSurfacePrivate()
{
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

void QWaylandShellSurfacePrivate::shell_surface_destroy_resource(Resource *)
{
    Q_Q(QWaylandShellSurface);

    delete q;
}

void QWaylandShellSurfacePrivate::shell_surface_move(Resource *resource,
                struct wl_resource *input_device_super,
                uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(QWaylandShellSurface);
    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(input_device_super);
    emit q->startMove(input_device);
}

void QWaylandShellSurfacePrivate::shell_surface_resize(Resource *resource,
                  struct wl_resource *input_device_super,
                  uint32_t serial,
                  uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(QWaylandShellSurface);

    QWaylandInputDevice *input_device = QWaylandInputDevice::fromSeatResource(input_device_super);
    emit q->startResize(input_device, QWaylandShellSurface::ResizeEdge(edges));
}

void QWaylandShellSurfacePrivate::shell_surface_set_toplevel(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandShellSurface);
    setFocusPolicy(QWaylandShellSurface::DefaultFocus);
    emit q->setDefaultToplevel();
}

void QWaylandShellSurfacePrivate::shell_surface_set_transient(Resource *resource,
                      struct wl_resource *parent_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(resource);
    Q_Q(QWaylandShellSurface);
    QWaylandSurface *parent_surface = QWaylandSurface::fromResource(parent_surface_resource);
    QWaylandShellSurface::FocusPolicy focusPolicy =
        flags & WL_SHELL_SURFACE_TRANSIENT_INACTIVE ? QWaylandShellSurface::NoKeyboardFocus
                                                    : QWaylandShellSurface::DefaultFocus;
    setFocusPolicy(focusPolicy);
    emit q->setTransient(parent_surface, QPoint(x,y), focusPolicy);
}

void QWaylandShellSurfacePrivate::shell_surface_set_fullscreen(Resource *resource,
                       uint32_t method,
                       uint32_t framerate,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);
    Q_UNUSED(method);
    Q_UNUSED(framerate);
    Q_Q(QWaylandShellSurface);
    setFocusPolicy(QWaylandShellSurface::DefaultFocus);
    QWaylandOutput *output = output_resource
            ? QWaylandOutput::fromResource(output_resource)
            : Q_NULLPTR;
    emit q->setFullScreen(QWaylandShellSurface::FullScreenMethod(method), framerate, output);
}

void QWaylandShellSurfacePrivate::shell_surface_set_popup(Resource *resource, wl_resource *input_device, uint32_t serial, wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_UNUSED(flags);
    Q_Q(QWaylandShellSurface);
    setFocusPolicy(QWaylandShellSurface::DefaultFocus);
    QWaylandInputDevice *input = QWaylandInputDevice::fromSeatResource(input_device);
    QWaylandSurface *parentSurface = QWaylandSurface::fromResource(parent);
    emit q->setPopup(input, parentSurface, QPoint(x,y));

}

void QWaylandShellSurfacePrivate::shell_surface_set_maximized(Resource *resource,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);
    Q_Q(QWaylandShellSurface);
    setFocusPolicy(QWaylandShellSurface::DefaultFocus);
    QWaylandOutput *output = output_resource
            ? QWaylandOutput::fromResource(output_resource)
            : Q_NULLPTR;
    emit q->setMaximized(output);
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
    if (title == m_title)
        return;
    Q_Q(QWaylandShellSurface);
    m_title = title;
    emit q->titleChanged();
}

void QWaylandShellSurfacePrivate::shell_surface_set_class(Resource *resource,
                             const QString &className)
{
    Q_UNUSED(resource);
    if (className == m_className)
        return;
    Q_Q(QWaylandShellSurface);
    m_className = className;
    emit q->classNameChanged();
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

QWaylandShellSurface::QWaylandShellSurface(QWaylandShell *shell, QWaylandSurface *surface, QWaylandClient *client, uint id)
    : QWaylandExtensionTemplate<QWaylandShellSurface>(*new QWaylandShellSurfacePrivate)
{
    initialize(shell, surface, client, id);
}

void QWaylandShellSurface::initialize(QWaylandShell *shell, QWaylandSurface *surface, QWaylandClient *client, uint id)
{
    Q_D(QWaylandShellSurface);
    d->m_shell = shell;
    d->m_surface = surface;
    d->init(client->client(), id, 1);
    setExtensionContainer(surface);
    emit surfaceChanged();
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

QSize QWaylandShellSurface::sizeForResize(const QSizeF &size, const QPointF &delta, QWaylandShellSurface::ResizeEdge edge)
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

void QWaylandShellSurface::sendConfigure(const QSize &size, ResizeEdge edges)
{
    Q_D(QWaylandShellSurface);
    d->send_configure(edges, size.width(), size.height());
}

void QWaylandShellSurface::sendPopupDone()
{
    Q_D(QWaylandShellSurface);
    d->send_popup_done();
}

QWaylandSurface *QWaylandShellSurface::surface() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_surface;
}

QWaylandShellSurface::FocusPolicy QWaylandShellSurface::focusPolicy() const
{
    Q_D(const QWaylandShellSurface);
    return d->m_focusPolicy;
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

QT_END_NAMESPACE
