// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDQTKEY_H
#define QWAYLANDQTKEY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qpa/qwindowsysteminterface.h>

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwayland-qt-key-unstable-v1.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLANDCLIENT_EXPORT QWaylandQtKeyExtension : public QtWayland::zqt_key_v1
{
public:
    QWaylandQtKeyExtension(QWaylandDisplay *display, uint32_t id);

private:
    QWaylandDisplay *m_display = nullptr;

    void zqt_key_v1_key(struct wl_surface *surface,
                        uint32_t time,
                        uint32_t type,
                        uint32_t key,
                        uint32_t modifiers,
                        uint32_t nativeScanCode,
                        uint32_t nativeVirtualKey,
                        uint32_t nativeModifiers,
                        const QString &text,
                        uint32_t autorep,
                        uint32_t count) override;

};

}

QT_END_NAMESPACE

#endif // QWAYLANDQTKEY_H
