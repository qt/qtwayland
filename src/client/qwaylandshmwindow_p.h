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

#ifndef QWAYLANDSHMWINDOW_H
#define QWAYLANDSHMWINDOW_H

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

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtGui/QRegion>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class Q_WAYLAND_CLIENT_EXPORT QWaylandShmWindow : public QWaylandWindow
{
public:
    QWaylandShmWindow(QWindow *window, QWaylandDisplay *display);
    ~QWaylandShmWindow() override;

    WindowType windowType() const override;
    QSurfaceFormat format() const override { return QSurfaceFormat(); }
};

}

QT_END_NAMESPACE

#endif // QWAYLANDSHMWINDOW_H
