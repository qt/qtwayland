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

#ifndef WAYLANDSURFACENODE_H
#define WAYLANDSURFACENODE_H

#include "waylandsurfacetexturematerial.h"

#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGOpaqueTextureMaterial>

class WaylandSurfaceItem;
class QSGTexture;

class WaylandSurfaceNode : public QSGGeometryNode
{
public:
    WaylandSurfaceNode(WaylandSurfaceItem *item = 0);
    ~WaylandSurfaceNode();

    void preprocess();
    void updateTexture();

    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h) { setRect(QRectF(x, y, w, h)); }

    void setTexture(QSGTexture *texture);
    QSGTexture *texture() const;

    bool isTextureUpdated() const { return m_textureUpdated; }
    void setTextureUpdated(bool textureUpdated) { m_textureUpdated = textureUpdated; }

    WaylandSurfaceItem *item() const { return m_item; }
    void setItem(WaylandSurfaceItem *item);

private:

    WaylandSurfaceItem *m_item;
    bool m_textureUpdated;
    bool m_useTextureAlpha;

    QSGGeometry m_geometry;
    QSGSimpleMaterial<WaylandSurfaceTextureState> *m_textureMaterial;
    QSGSimpleMaterial<WaylandSurfaceTextureState> *m_opaqueTextureMaterial;
    QSGSimpleMaterial<WaylandSurfaceTextureState> *m_currentMaterial;

    QRectF m_rect;
};

#endif // WAYLANDSURFACENODE_H
