// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandqtkey_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylanddisplay_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandQtKeyExtension::QWaylandQtKeyExtension(QWaylandDisplay *display, uint32_t id)
    : QtWayland::zqt_key_v1(display->wl_registry(), id, 1)
    , m_display(display)
{
}

void QWaylandQtKeyExtension::zqt_key_v1_key(struct wl_surface *surface,
                                                 uint32_t time,
                                                 uint32_t type,
                                                 uint32_t key,
                                                 uint32_t modifiers,
                                                 uint32_t nativeScanCode,
                                                 uint32_t nativeVirtualKey,
                                                 uint32_t nativeModifiers,
                                                 const QString &text,
                                                 uint32_t autorep,
                                                 uint32_t count)
{
    QList<QWaylandInputDevice *> inputDevices = m_display->inputDevices();
    if (!surface && inputDevices.isEmpty()) {
        qWarning("qt_key_extension: handle_qtkey: No input device");
        return;
    }

    QWaylandInputDevice *dev = inputDevices.first();

    auto *win = surface ? QWaylandWindow::fromWlSurface(surface) : nullptr;

    if (!win)
        win = dev->keyboardFocus();

    if (!win || !win->window()) {
        qWarning("qt_key_extension: handle_qtkey: No keyboard focus");
        return;
    }

    QWindow *window = win->window();
    QWindowSystemInterface::handleExtendedKeyEvent(window, time, QEvent::Type(type), key, Qt::KeyboardModifiers(modifiers),
                                                   nativeScanCode, nativeVirtualKey, nativeModifiers, text,
                                                   autorep, count);
}

}

QT_END_NAMESPACE
