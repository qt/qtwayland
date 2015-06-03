/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef QQUICKWAYLANDSURFACE_H
#define QQUICKWAYLANDSURFACE_H

#include <QtCompositor/qwaylandsurface.h>

struct wl_client;

QT_BEGIN_NAMESPACE

class QSGTexture;

class QWaylandSurfaceItem;
class QWaylandQuickSurfacePrivate;
class QWaylandQuickCompositor;

class Q_COMPOSITOR_EXPORT QWaylandQuickSurface : public QWaylandSurface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickSurface)
    Q_PROPERTY(bool useTextureAlpha READ useTextureAlpha WRITE setUseTextureAlpha NOTIFY useTextureAlphaChanged)
    Q_PROPERTY(bool clientRenderingEnabled READ clientRenderingEnabled WRITE setClientRenderingEnabled NOTIFY clientRenderingEnabledChanged)
    Q_PROPERTY(QObject *windowProperties READ windowPropertyMap CONSTANT)
public:
    QWaylandQuickSurface(wl_client *client, quint32 id, int version, QWaylandQuickCompositor *compositor);
    ~QWaylandQuickSurface();

    QSGTexture *texture() const;

    bool useTextureAlpha() const;
    void setUseTextureAlpha(bool useTextureAlpha);

    bool clientRenderingEnabled() const;
    void setClientRenderingEnabled(bool enabled);

    QObject *windowPropertyMap() const;

private:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void useTextureAlphaChanged();
    void clientRenderingEnabledChanged();

private:
    void updateTexture();
    void invalidateTexture();

};

QT_END_NAMESPACE

#endif
