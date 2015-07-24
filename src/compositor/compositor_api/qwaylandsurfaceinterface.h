/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef QWAYLANDSURFACEINTERFACE_H
#define QWAYLANDSURFACEINTERFACE_H

#include <QWindow>

#include <QtCompositor/qwaylandsurface.h>
#include <QtCompositor/qwaylandexport.h>

QT_BEGIN_NAMESPACE

class QWaylandSurface;

class Q_COMPOSITOR_EXPORT QWaylandSurfaceOp
{
public:
    enum Type {
        Close,
        SetVisibility,
        Resize,
        Ping,
        UserType = 1000
    };

    QWaylandSurfaceOp(int t);
    virtual ~QWaylandSurfaceOp();

    int type() const;

private:
    class Private;
    Private *const d;
};

class Q_COMPOSITOR_EXPORT QWaylandSurfaceSetVisibilityOp : public QWaylandSurfaceOp
{
public:
    QWaylandSurfaceSetVisibilityOp(QWindow::Visibility visibility);
    QWindow::Visibility visibility() const;

private:
    QWindow::Visibility m_visibility;
};

class Q_COMPOSITOR_EXPORT QWaylandSurfaceResizeOp : public QWaylandSurfaceOp
{
public:
    QWaylandSurfaceResizeOp(const QSize &size);
    QSize size() const;

private:
    QSize m_size;
};

class Q_COMPOSITOR_EXPORT QWaylandSurfacePingOp : public QWaylandSurfaceOp
{
public:
    QWaylandSurfacePingOp(quint32 serial);
    quint32 serial() const;

private:
    quint32 m_serial;
};

class Q_COMPOSITOR_EXPORT QWaylandSurfaceInterface
{
public:
    QWaylandSurfaceInterface(QWaylandSurface *surf);
    virtual ~QWaylandSurfaceInterface();

    QWaylandSurface *surface() const;

protected:
    virtual bool runOperation(QWaylandSurfaceOp *op) = 0;

    void setSurfaceType(QWaylandSurface::WindowType type);
    void setSurfaceClassName(const QString &name);
    void setSurfaceTitle(const QString &title);

private:
    class Private;
    Private *const d;
    friend class QWaylandSurface;
};

QT_END_NAMESPACE

#endif
