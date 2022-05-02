/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QWAYLANDSUBSURFACE_H
#define QWAYLANDSUBSURFACE_H

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

#include <QtCore/qglobal.h>
#include <QtCore/qmutex.h>

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwayland-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandSubSurface : public QtWayland::wl_subsurface
{
public:
    QWaylandSubSurface(QWaylandWindow *window, QWaylandWindow *parent, ::wl_subsurface *subsurface);
    ~QWaylandSubSurface() override;

    QWaylandWindow *window() const { return m_window; }
    QWaylandWindow *parent() const { return m_parent; }

    void setSync();
    void setDeSync();
    bool isSync() const { return m_synchronized; }
    QMutex *syncMutex() { return &m_syncLock; }

private:

    // Intentionally hide public methods from ::wl_subsurface
    // to keep track of the sync state
    void set_sync();
    void set_desync();
    QWaylandWindow *m_window = nullptr;
    QWaylandWindow *m_parent = nullptr;
    bool m_synchronized = false;
    QMutex m_syncLock;

};

QT_END_NAMESPACE

}

#endif // QWAYLANDSUBSURFACE_H
