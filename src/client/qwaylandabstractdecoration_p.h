/****************************************************************************
**
** Copyright (C) 2014 Robin Burchell <robin.burchell@viroteck.net>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
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
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDABSTRACTDECORATION_H
#define QWAYLANDABSTRACTDECORATION_H

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

#include <QtCore/QMargins>
#include <QtCore/QPointF>
#include <QtGui/QGuiApplication>
#include <QtGui/QCursor>
#include <QtGui/QColor>
#include <QtGui/QStaticText>
#include <QtGui/QImage>
#include <QtWaylandClient/private/qwaylandclientexport_p.h>

#include <wayland-client.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

class QWindow;
class QPaintDevice;
class QPainter;
class QEvent;

namespace QtWaylandClient {

class QWaylandScreen;
class QWaylandWindow;
class QWaylandInputDevice;
class QWaylandAbstractDecorationPrivate;

class Q_WAYLAND_CLIENT_EXPORT QWaylandAbstractDecoration : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandAbstractDecoration)
public:
    QWaylandAbstractDecoration();
    virtual ~QWaylandAbstractDecoration();

    void setWaylandWindow(QWaylandWindow *window);
    QWaylandWindow *waylandWindow() const;

    void update();
    bool isDirty() const;

    virtual QMargins margins() const = 0;
    QWindow *window() const;
    const QImage &contentImage();

    virtual bool handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,Qt::MouseButtons b,Qt::KeyboardModifiers mods) = 0;
    virtual bool handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::TouchPointState state, Qt::KeyboardModifiers mods) = 0;

protected:
    virtual void paint(QPaintDevice *device) = 0;

    void setMouseButtons(Qt::MouseButtons mb);

    void startResize(QWaylandInputDevice *inputDevice,enum wl_shell_surface_resize resize, Qt::MouseButtons buttons);
    void startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons);

    bool isLeftClicked(Qt::MouseButtons newMouseButtonState);
    bool isLeftReleased(Qt::MouseButtons newMouseButtonState);
};

}

QT_END_NAMESPACE

#endif // QWAYLANDABSTRACTDECORATION_H
