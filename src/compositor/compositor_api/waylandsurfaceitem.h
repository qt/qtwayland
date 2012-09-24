/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WAYLANDSURFACEITEM_H
#define WAYLANDSURFACEITEM_H

#include "waylandexport.h"
#include "waylandsurface.h"

#include <QtQuick/QQuickItem>
#include <QtQuick/qsgtexture.h>

#include <QtQuick/qsgtextureprovider.h>

class WaylandSurfaceTextureProvider;
class WaylandSurfaceNode;
class QMutex;

Q_DECLARE_METATYPE(WaylandSurface*)

class Q_COMPOSITOR_EXPORT WaylandSurfaceItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(WaylandSurface* surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(bool paintEnabled READ paintEnabled WRITE setPaintEnabled)
    Q_PROPERTY(bool useTextureAlpha READ useTextureAlpha WRITE setUseTextureAlpha NOTIFY useTextureAlphaChanged)
    Q_PROPERTY(bool clientRenderingEnabled READ clientRenderingEnabled WRITE setClientRenderingEnabled NOTIFY clientRenderingEnabledChanged)
    Q_PROPERTY(bool touchEventsEnabled READ touchEventsEnabled WRITE setTouchEventsEnabled NOTIFY touchEventsEnabledChanged)
    Q_PROPERTY(bool isYInverted READ isYInverted NOTIFY yInvertedChanged)

public:
    WaylandSurfaceItem(QQuickItem *parent = 0);
    WaylandSurfaceItem(WaylandSurface *surface, QQuickItem *parent = 0);
    ~WaylandSurfaceItem();

    void setSurface(WaylandSurface *surface);
    WaylandSurface *surface() const {return m_surface; }

    Q_INVOKABLE bool isYInverted() const;

    bool isTextureProvider() const { return true; }
    QSGTextureProvider *textureProvider() const;

    bool paintEnabled() const;
    bool useTextureAlpha() const  { return m_useTextureAlpha; }
    bool clientRenderingEnabled() const { return m_clientRenderingEnabled; }
    bool touchEventsEnabled() const { return m_touchEventsEnabled; }

    void setUseTextureAlpha(bool useTextureAlpha);
    void setClientRenderingEnabled(bool enabled);
    void setTouchEventsEnabled(bool enabled);

    void setDamagedFlag(bool on);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void touchEvent(QTouchEvent *event);

public slots:
    void takeFocus();
    void setPaintEnabled(bool paintEnabled);

private slots:
    void surfaceMapped();
    void surfaceUnmapped();
    void surfaceDestroyed(QObject *object);
    void surfaceDamaged(const QRect &);
    void parentChanged(WaylandSurface *newParent, WaylandSurface *oldParent);
    void updateSize();
    void updatePosition();

signals:
    void textureChanged();
    void useTextureAlphaChanged();
    void clientRenderingEnabledChanged();
    void touchEventsEnabledChanged();
    void yInvertedChanged();
    void surfaceChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private:
    friend class WaylandSurfaceNode;
    void updateTexture();
    QPoint toSurface(const QPointF &pos) const;
    void init(WaylandSurface *);
    void ensureProvider();

    static QMutex *mutex;

    WaylandSurface *m_surface;
    WaylandSurfaceTextureProvider *m_provider;
    WaylandSurfaceNode *m_node;
    bool m_paintEnabled;
    bool m_useTextureAlpha;
    bool m_clientRenderingEnabled;
    bool m_touchEventsEnabled;
    bool m_damaged;
    bool m_yInverted;
};

#endif
