/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QWAYLANDXDGDECORATIONV1_P_H
#define QWAYLANDXDGDECORATIONV1_P_H

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

#include "qwayland-xdg-decoration-unstable-v1.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandXdgToplevel;
class QWaylandXdgToplevelDecorationV1;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgDecorationManagerV1 : public QtWayland::zxdg_decoration_manager_v1
{
public:
    QWaylandXdgDecorationManagerV1(struct ::wl_registry *registry, uint32_t id, uint32_t availableVersion);
    ~QWaylandXdgDecorationManagerV1() override;
    QWaylandXdgToplevelDecorationV1 *createToplevelDecoration(::xdg_toplevel *toplevel);
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgToplevelDecorationV1 : public QtWayland::zxdg_toplevel_decoration_v1
{
public:
    QWaylandXdgToplevelDecorationV1(::zxdg_toplevel_decoration_v1 *decoration);
    ~QWaylandXdgToplevelDecorationV1() override;
    void requestMode(mode mode);
    void unsetMode();
    mode pending() const;
    bool isConfigured() const;

protected:
    void zxdg_toplevel_decoration_v1_configure(uint32_t mode) override;

private:
    mode m_pending = mode_client_side;
    mode m_requested = mode_client_side;
    bool m_modeSet = false;
    bool m_configured = false;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDXDGDECORATIONV1_P_H
