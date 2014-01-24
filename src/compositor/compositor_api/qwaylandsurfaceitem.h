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

#ifndef QWAYLANDSURFACEITEM_H
#define QWAYLANDSURFACEITEM_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandsurface.h>

#include <QtQuick/QQuickItem>
#include <QtQuick/qsgtexture.h>

#include <QtQuick/qsgtextureprovider.h>

QT_BEGIN_NAMESPACE

class QWaylandSurfaceTextureProvider;
class QMutex;

Q_DECLARE_METATYPE(QWaylandSurface*)

class Q_COMPOSITOR_EXPORT QWaylandSurfaceItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QWaylandSurface* surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(bool paintEnabled READ paintEnabled WRITE setPaintEnabled)
    Q_PROPERTY(bool useTextureAlpha READ useTextureAlpha WRITE setUseTextureAlpha NOTIFY useTextureAlphaChanged)
    Q_PROPERTY(bool clientRenderingEnabled READ clientRenderingEnabled WRITE setClientRenderingEnabled NOTIFY clientRenderingEnabledChanged)
    Q_PROPERTY(bool touchEventsEnabled READ touchEventsEnabled WRITE setTouchEventsEnabled NOTIFY touchEventsEnabledChanged)
    Q_PROPERTY(bool isYInverted READ isYInverted NOTIFY yInvertedChanged)
    Q_PROPERTY(bool resizeSurfaceToItem READ resizeSurfaceToItem WRITE setResizeSurfaceToItem NOTIFY resizeSurfaceToItemChanged)

public:
    QWaylandSurfaceItem(QQuickItem *parent = 0);
    QWaylandSurfaceItem(QWaylandSurface *surface, QQuickItem *parent = 0);
    ~QWaylandSurfaceItem();

    void setSurface(QWaylandSurface *surface);
    QWaylandSurface *surface() const {return m_surface; }

    Q_INVOKABLE bool isYInverted() const;

    bool isTextureProvider() const { return true; }
    QSGTextureProvider *textureProvider() const;

    bool paintEnabled() const;
    bool useTextureAlpha() const  { return m_useTextureAlpha; }
    bool clientRenderingEnabled() const { return m_clientRenderingEnabled; }
    bool touchEventsEnabled() const { return m_touchEventsEnabled; }
    bool resizeSurfaceToItem() const { return m_resizeSurfaceToItem; }

    void setUseTextureAlpha(bool useTextureAlpha);
    void setClientRenderingEnabled(bool enabled);
    void setTouchEventsEnabled(bool enabled);
    void setResizeSurfaceToItem(bool enabled);

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
    void parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent);
    void updateSize();
    void updateSurfaceSize();
    void updatePosition();

signals:
    void textureChanged();
    void useTextureAlphaChanged();
    void clientRenderingEnabledChanged();
    void touchEventsEnabledChanged();
    void yInvertedChanged();
    void surfaceChanged();
    void resizeSurfaceToItemChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private:
    friend class QWaylandSurfaceNode;
    void updateTexture();
    void init(QWaylandSurface *);
    void ensureProvider();

    static QMutex *mutex;

    QWaylandSurface *m_surface;
    QWaylandSurfaceTextureProvider *m_provider;
    bool m_paintEnabled;
    bool m_mapped;
    bool m_useTextureAlpha;
    bool m_clientRenderingEnabled;
    bool m_touchEventsEnabled;
    bool m_damaged;
    bool m_yInverted;
    bool m_resizeSurfaceToItem;
};

QT_END_NAMESPACE

#endif
