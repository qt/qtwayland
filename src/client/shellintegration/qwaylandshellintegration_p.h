/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef QWAYLANDSHELLINTEGRATION_H
#define QWAYLANDSHELLINTEGRATION_H

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

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandDisplay;
class QWaylandShellSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShellIntegration
{
public:
    QWaylandShellIntegration() {}
    virtual ~QWaylandShellIntegration() {}

    virtual bool initialize(QWaylandDisplay *display) {
        m_display = display;
        return true;
    }
    virtual QWaylandShellSurface *createShellSurface(QWaylandWindow *window) = 0;
    virtual void handleKeyboardFocusChanged(QWaylandWindow *newFocus, QWaylandWindow *oldFocus) {
        if (newFocus)
            m_display->handleWindowActivated(newFocus);
        if (oldFocus)
            m_display->handleWindowDeactivated(oldFocus);
    }
    virtual void *nativeResourceForWindow(const QByteArray &resource, QWindow *window) {
        Q_UNUSED(resource);
        Q_UNUSED(window);
        return nullptr;
    }

protected:
    QWaylandDisplay *m_display = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDSHELLINTEGRATION_H
