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

#ifndef QWAYLANDSURFACEITEM_H
#define QWAYLANDSURFACEITEM_H

#include <QtCompositor/qwaylandexport.h>

#include <QtQuick/QQuickItem>
#include <QtQuick/qsgtexture.h>

#include <QtQuick/qsgtextureprovider.h>

#include <QtCompositor/qwaylandsurfaceview.h>
#include <QtCompositor/qwaylandquicksurface.h>

Q_DECLARE_METATYPE(QWaylandQuickSurface*)

QT_BEGIN_NAMESPACE

class QWaylandSurfaceTextureProvider;
class QMutex;
class QWaylandInputDevice;

class Q_COMPOSITOR_EXPORT QWaylandSurfaceItem : public QQuickItem, public QWaylandSurfaceView
{
    Q_OBJECT
    Q_PROPERTY(QWaylandQuickSurface* surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(bool paintEnabled READ paintEnabled WRITE setPaintEnabled)
    Q_PROPERTY(bool touchEventsEnabled READ touchEventsEnabled WRITE setTouchEventsEnabled NOTIFY touchEventsEnabledChanged)
    Q_PROPERTY(QWaylandSurface::Origin origin READ origin NOTIFY originChanged);
    Q_PROPERTY(bool resizeSurfaceToItem READ resizeSurfaceToItem WRITE setResizeSurfaceToItem NOTIFY resizeSurfaceToItemChanged)
    Q_PROPERTY(bool followRequestedPosition READ followRequestedPosition WRITE setFollowRequestedPosition NOTIFY followRequestedPositionChanged)
    Q_PROPERTY(qreal requestedXPosition READ requestedXPosition WRITE setRequestedXPosition NOTIFY requestedXPositionChanged)
    Q_PROPERTY(qreal requestedYPosition READ requestedYPosition WRITE setRequestedYPosition NOTIFY requestedYPositionChanged)
    Q_PROPERTY(bool inputEventsEnabled READ inputEventsEnabled WRITE setInputEventsEnabled NOTIFY inputEventsEnabledChanged)
    Q_PROPERTY(bool lockedBuffer READ lockedBuffer WRITE setLockedBuffer NOTIFY lockedBufferChanged)

public:
    QWaylandSurfaceItem(QQuickItem *parent = 0);
    ~QWaylandSurfaceItem();

    QWaylandQuickSurface *surface() const;
    void setSurface(QWaylandQuickSurface *surface);

    QWaylandSurface::Origin origin() const;

    bool isTextureProvider() const { return true; }
    QSGTextureProvider *textureProvider() const;

    bool paintEnabled() const;
    bool touchEventsEnabled() const { return m_touchEventsEnabled; }
    bool resizeSurfaceToItem() const { return m_resizeSurfaceToItem; }

    void setTouchEventsEnabled(bool enabled);
    void setResizeSurfaceToItem(bool enabled);

    bool inputEventsEnabled() const { return m_inputEventsEnabled; }
    void setInputEventsEnabled(bool enabled);

    void setRequestedPosition(const QPointF &pos) Q_DECL_OVERRIDE;
    QPointF pos() const Q_DECL_OVERRIDE;

    bool followRequestedPosition() const;
    void setFollowRequestedPosition(bool follow);
    qreal requestedXPosition() const;
    void setRequestedXPosition(qreal xPos);
    qreal requestedYPosition() const;
    void setRequestedYPosition(qreal yPos);

    Q_INVOKABLE void syncGraphicsState();

    bool lockedBuffer() const;
    void setLockedBuffer(bool locked);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void hoverEnterEvent(QHoverEvent *event);
    void hoverMoveEvent(QHoverEvent *event);
    void wheelEvent(QWheelEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void touchEvent(QTouchEvent *event);
    void mouseUngrabEvent() Q_DECL_OVERRIDE;

    void waylandSurfaceChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface) Q_DECL_OVERRIDE;
    void waylandSurfaceDestroyed() Q_DECL_OVERRIDE;

    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) Q_DECL_OVERRIDE;
public Q_SLOTS:
    virtual void takeFocus(QWaylandInputDevice *device = 0);
    void setPaintEnabled(bool paintEnabled);

private Q_SLOTS:
    void surfaceMapped();
    void surfaceUnmapped();
    void parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent);
    void updateSize();
    void updateBuffer(bool hasBuffer);
    void updateWindow();
    void beforeSync();

Q_SIGNALS:
    void surfaceChanged();
    void touchEventsEnabledChanged();
    void originChanged();
    void resizeSurfaceToItemChanged();
    void surfaceDestroyed();
    void followRequestedPositionChanged();
    void requestedXPositionChanged();
    void requestedYPositionChanged();
    void inputEventsEnabledChanged();
    void lockedBufferChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private:
    friend class QWaylandSurfaceNode;
    friend class QWaylandQuickSurface;
    bool shouldSendInputEvents() const { return surface() && m_inputEventsEnabled; }

    static QMutex *mutex;

    mutable QWaylandSurfaceTextureProvider *m_provider;
    bool m_paintEnabled;
    bool m_touchEventsEnabled;
    bool m_resizeSurfaceToItem;
    bool m_followRequestedPos;
    bool m_inputEventsEnabled;
    bool m_newTexture;

    QQuickWindow *m_connectedWindow;
    QWaylandSurface::Origin m_origin;
};

QT_END_NAMESPACE

#endif
