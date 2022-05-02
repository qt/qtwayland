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

#ifndef QWAYLANDSURFACE_P_H
#define QWAYLANDSURFACE_P_H

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

#include <QtGui/QScreen>

#include <QtWaylandClient/private/qwayland-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandScreen;
class QWaylandWindow;
class QWaylandDisplay;

class QWaylandSurface : public QObject, public QtWayland::wl_surface
{
    Q_OBJECT
public:
    explicit QWaylandSurface(QWaylandDisplay *display);
    ~QWaylandSurface() override;
    QWaylandScreen *oldestEnteredScreen();
    QWaylandWindow *waylandWindow() const { return m_window; }

    static QWaylandSurface *fromWlSurface(::wl_surface *surface);

signals:
    void screensChanged();

private slots:
    void handleScreenRemoved(QScreen *qScreen);

protected:
    void surface_enter(struct ::wl_output *output) override;
    void surface_leave(struct ::wl_output *output) override;

    QList<QWaylandScreen *> m_screens; //As seen by wl_surface.enter/leave events. Chronological order.
    QWaylandWindow *m_window = nullptr;

    friend class QWaylandWindow; // TODO: shouldn't need to be friends
};

} // namespace QtWaylandClient

QT_END_NAMESPACE

#endif // QWAYLANDSURFACE_P_H
