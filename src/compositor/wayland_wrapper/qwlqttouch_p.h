/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef WLTOUCH_H
#define WLTOUCH_H

#include <private/qwlcompositor_p.h>
#include <QtCompositor/private/qwayland-server-touch-extension.h>
#include "wayland-util.h"

QT_BEGIN_NAMESPACE

class Compositor;
class Surface;
class QTouchEvent;
class QWaylandSurfaceView;

namespace QtWayland {

class TouchExtensionGlobal : public QtWaylandServer::qt_touch_extension
{
public:
    TouchExtensionGlobal(Compositor *compositor);
    ~TouchExtensionGlobal();

    bool postTouchEvent(QTouchEvent *event, QWaylandSurfaceView *view);

    void setFlags(int flags) { m_flags = flags; }

protected:
    void touch_extension_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
    void touch_extension_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

private:
    Compositor *m_compositor;
    int m_flags;
    QList<Resource *> m_resources;
    QVector<float> m_posData;
};

}

QT_END_NAMESPACE

#endif // WLTOUCH_H
