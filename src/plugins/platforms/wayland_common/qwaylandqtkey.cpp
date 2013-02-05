/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandqtkey.h"
#include "qwaylandinputdevice.h"

#include "wayland-qtkey-extension-client-protocol.h"

QT_USE_NAMESPACE

QWaylandQtKeyExtension::QWaylandQtKeyExtension(QWaylandDisplay *display, uint32_t id)
    : m_display(display)
{
    m_qtkey = static_cast<struct wl_qtkey_extension *>(wl_registry_bind(display->wl_registry(), id, &wl_qtkey_extension_interface, 1));
    wl_qtkey_extension_add_listener(m_qtkey, &qtkey_listener, this);
}

void QWaylandQtKeyExtension::handle_qtkey(void *data,
                                          struct wl_qtkey_extension *ext,
                                          uint32_t time,
                                          uint32_t type,
                                          uint32_t key,
                                          uint32_t modifiers,
                                          uint32_t nativeScanCode,
                                          uint32_t nativeVirtualKey,
                                          uint32_t nativeModifiers,
                                          const char *text,
                                          uint32_t autorep,
                                          uint32_t count)
{
    Q_UNUSED(ext);
    QWaylandQtKeyExtension *self = static_cast<QWaylandQtKeyExtension *>(data);

    QList<QWaylandInputDevice *> inputDevices = self->m_display->inputDevices();
    if (inputDevices.isEmpty()) {
        qWarning("wl_qtkey_extension: handle_qtkey: No input device");
        return;
    }

    QWaylandInputDevice *dev = inputDevices.first();
    QWaylandWindow *win = dev->mKeyboardFocus;

    if (!win || !win->window()) {
        qWarning("wl_qtkey_extension: handle_qtkey: No keyboard focus");
        return;
    }

    QWindow *window = win->window();
    QWindowSystemInterface::handleExtendedKeyEvent(window, time, QEvent::Type(type), key, Qt::KeyboardModifiers(modifiers),
                                                   nativeScanCode, nativeVirtualKey, nativeModifiers, QString::fromUtf8(text),
                                                   autorep, count);
}

const struct wl_qtkey_extension_listener QWaylandQtKeyExtension::qtkey_listener = {
    QWaylandQtKeyExtension::handle_qtkey
};

