/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDDECORATION_H
#define QWAYLANDDECORATION_H

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
class QWaylandScreen;
class QWaylandWindow;
class QWaylandInputDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDecoration
{
public:
    QWaylandDecoration(QWaylandWindow *window);
    virtual ~QWaylandDecoration();

    void update();
    bool isDirty() const;

    bool handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    bool inMouseButtonPressedState() const;

    void startResize(QWaylandInputDevice *inputDevice,enum wl_shell_surface_resize resize, Qt::MouseButtons buttons);
    void startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons);
    QMargins margins() const;
    QWindow *window() const;
    QWaylandWindow *waylandWindow() const;
    const QImage &contentImage();

    void setForegroundColor(const QColor &c);
    inline QColor foregroundColor() const;

    void setBackgroundColor(const QColor &c);
    inline QColor backgroundColor() const;

protected:
    void paint(QPaintDevice *device);

private:
    void processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);

    bool isLeftClicked(Qt::MouseButtons newMouseButtonState);
    bool isLeftReleased(Qt::MouseButtons newMouseButtonState);

    QRectF closeButtonRect() const;
    QRectF maximizeButtonRect() const;
    QRectF minimizeButtonRect() const;

    QWindow *m_window;
    QWaylandWindow *m_wayland_window;

    bool m_isDirty;
    QImage m_decorationContentImage;

    QMargins m_margins;
    Qt::MouseButtons m_mouseButtons;

    QColor m_foregroundColor;
    QColor m_backgroundColor;
    QStaticText m_windowTitle;
};

inline bool QWaylandDecoration::isDirty() const
{
    return m_isDirty;
}

inline QMargins QWaylandDecoration::margins() const
{
    return m_margins;
}

inline QWindow *QWaylandDecoration::window() const
{
    return m_window;
}

inline QWaylandWindow *QWaylandDecoration::waylandWindow() const
{
    return m_wayland_window;
}

inline QColor QWaylandDecoration::foregroundColor() const
{
    return m_foregroundColor;
}

inline QColor QWaylandDecoration::backgroundColor() const
{
    return m_backgroundColor;
}

QT_END_NAMESPACE

#endif // QWAYLANDDECORATION_H
