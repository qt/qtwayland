// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WLTOUCH_H
#define WLTOUCH_H

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

#include <QtWaylandCompositor/private/qwayland-server-touch-extension.h>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtCore/private/qglobal_p.h>

#include <wayland-util.h>

QT_BEGIN_NAMESPACE

class Surface;
class QTouchEvent;
class QWaylandView;

namespace QtWayland {

class TouchExtensionGlobal : public QWaylandCompositorExtensionTemplate<TouchExtensionGlobal>, public QtWaylandServer::qt_touch_extension
{
    Q_OBJECT
    Q_PROPERTY(BehaviorFlags behaviorFlags READ behaviorFlags WRITE setBehviorFlags NOTIFY behaviorFlagsChanged)
public:

    enum BehaviorFlag{
        None = 0x00,
        MouseFromTouch = 0x01
    };
    Q_DECLARE_FLAGS(BehaviorFlags, BehaviorFlag)

    TouchExtensionGlobal(QWaylandCompositor *compositor);
    ~TouchExtensionGlobal() override;

    bool postTouchEvent(QTouchEvent *event, QWaylandSurface *surface);

    void setBehviorFlags(BehaviorFlags flags);
    BehaviorFlags behaviorFlags() const { return m_flags; }

Q_SIGNALS:
    void behaviorFlagsChanged();

protected:
    void touch_extension_bind_resource(Resource *resource) override;
    void touch_extension_destroy_resource(Resource *resource) override;

private:
    QWaylandCompositor *m_compositor = nullptr;
    BehaviorFlags m_flags = BehaviorFlag::None;
    QList<Resource *> m_resources;
    QList<float> m_posData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TouchExtensionGlobal::BehaviorFlags)

}

QT_END_NAMESPACE

#endif // WLTOUCH_H
