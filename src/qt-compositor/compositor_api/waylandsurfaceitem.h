/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#ifndef WAYLANDSURFACEITEM_H
#define WAYLANDSURFACEITEM_H

#include <QSGItem>
#include <qsgtexture.h>

#include <private/qsgtextureprovider_p.h>

class WaylandSurface;
Q_DECLARE_METATYPE(WaylandSurface*)

class WaylandSurfaceItem : public QSGItem, public QSGTextureProvider
{
    Q_OBJECT
    Q_INTERFACES(QSGTextureProvider)
    Q_PROPERTY(WaylandSurface* surface READ surface WRITE setSurface)
    Q_PROPERTY(bool paintEnabled READ paintEnabled WRITE setPaintEnabled)
    Q_PROPERTY(bool useTextureAlpha READ useTextureAlpha WRITE setUseTextureAlpha NOTIFY useTextureAlphaChanged)
    Q_PROPERTY(bool clientRenderingEnabled READ clientRenderingEnabled WRITE setClientRenderingEnabled NOTIFY clientRenderingEnabledChanged)

public:
    WaylandSurfaceItem(QSGItem *parent = 0);
    WaylandSurfaceItem(WaylandSurface *surface, QSGItem *parent = 0);
    ~WaylandSurfaceItem();

    void setSurface(WaylandSurface *surface);
    WaylandSurface *surface() const {return m_surface; }

    Q_INVOKABLE bool isYInverted() const;

    QSGTexture *texture() const;
    const char *textureChangedSignal() const { return SIGNAL(textureChanged()); }

    bool paintEnabled() const;
    bool useTextureAlpha() const  { return m_useTextureAlpha; }
    bool clientRenderingEnabled() const { return m_clientRenderingEnabled; }

    void setUseTextureAlpha(bool useTextureAlpha);
    void setClientRenderingEnabled(bool enabled);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void takeFocus();
    void setPaintEnabled(bool paintEnabled);

private slots:
    void surfaceMapped(const QSize &size);
    void surfaceDestroyed(QObject *object);
    void surfaceDamaged(const QRect &);

signals:
    void textureChanged();
    void useTextureAlphaChanged();
    void clientRenderingEnabledChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private:
    QPoint toSurface(const QPointF &pos) const;
    void init(WaylandSurface *);

    WaylandSurface *m_surface;
    QSGTexture *m_texture;
    bool m_paintEnabled;
    bool m_useTextureAlpha;
    bool m_clientRenderingEnabled;
};

#endif
