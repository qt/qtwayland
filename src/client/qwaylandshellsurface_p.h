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

#ifndef QWAYLANDSHELLSURFACE_H
#define QWAYLANDSHELLSURFACE_H

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

#include <QtCore/QSize>
#include <QObject>

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

class QVariant;
class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShellSurface : public QObject
{
    Q_OBJECT
public:
    explicit QWaylandShellSurface(QWaylandWindow *window);
    ~QWaylandShellSurface() override {}
    virtual bool resize(QWaylandInputDevice *, Qt::Edges) { return false; }
    virtual bool move(QWaylandInputDevice *) { return false; }
    virtual bool showWindowMenu(QWaylandInputDevice *seat) { Q_UNUSED(seat); return false; }
    virtual void setTitle(const QString & /*title*/) {}
    virtual void setAppId(const QString & /*appId*/) {}

    virtual void setWindowFlags(Qt::WindowFlags flags);

    virtual bool isExposed() const { return true; }
    virtual bool handleExpose(const QRegion &) { return false; }

    virtual void raise() {}
    virtual void lower() {}
    virtual void setContentOrientationMask(Qt::ScreenOrientations orientation) { Q_UNUSED(orientation); }

    virtual void sendProperty(const QString &name, const QVariant &value);

    inline QWaylandWindow *window() { return m_window; }

    virtual void applyConfigure() {}
    virtual void requestWindowStates(Qt::WindowStates states) {Q_UNUSED(states);}
    virtual bool wantsDecorations() const { return false; }

    virtual void propagateSizeHints() {}

    virtual void setWindowGeometry(const QRect &rect) { Q_UNUSED(rect); }

private:
    QWaylandWindow *m_window = nullptr;
    friend class QWaylandWindow;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDSHELLSURFACE_H
