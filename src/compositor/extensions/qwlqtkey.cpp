// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlqtkey_p.h"
#include <QtWaylandCompositor/QWaylandSurface>
#include <QKeyEvent>
#include <QWindow>

QT_BEGIN_NAMESPACE

namespace QtWayland {

QtKeyExtensionGlobal::QtKeyExtensionGlobal(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(compositor)
    , QtWaylandServer::zqt_key_v1(compositor->display(), 1)
    , m_compositor(compositor)
{
}

bool QtKeyExtensionGlobal::postQtKeyEvent(QKeyEvent *event, QWaylandSurface *surface)
{
    uint32_t time = m_compositor->currentTimeMsecs();

    Resource *target = surface ? resourceMap().value(surface->waylandClient()) : 0;

    if (target) {
        send_key(target->handle,
                 surface ? surface->resource() : nullptr,
                 time, event->type(), event->key(), event->modifiers(),
                 event->nativeScanCode(),
                 event->nativeVirtualKey(),
                 event->nativeModifiers(),
                 event->text(),
                 event->isAutoRepeat(),
                 event->count());

        return true;
    }

    return false;
}

}

QT_END_NAMESPACE

#include "moc_qwlqtkey_p.cpp"
