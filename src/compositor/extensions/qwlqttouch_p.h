/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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
****************************************************************************/

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
    QVector<float> m_posData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TouchExtensionGlobal::BehaviorFlags)

}

QT_END_NAMESPACE

#endif // WLTOUCH_H
