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

#include "qwaylandsurfacenode_p.h"
#include "qwaylandsurfaceitem.h"

#include <QtCore/QMutexLocker>
#include <QtQuick/QSGTexture>
#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QSGFlatColorMaterial>

QT_BEGIN_NAMESPACE

QWaylandSurfaceNode::QWaylandSurfaceNode(QWaylandSurfaceItem *item)
    : m_item(item)
    , m_textureUpdated(false)
    , m_useTextureAlpha(false)
{
    if (m_item)
        m_item->m_node = this;
    setFlag(UsePreprocess,true);
}


QWaylandSurfaceNode::~QWaylandSurfaceNode()
{
    QMutexLocker locker(QWaylandSurfaceItem::mutex);
    if (m_item)
        m_item->m_node = 0;
}

void QWaylandSurfaceNode::preprocess()
{
    QMutexLocker locker(QWaylandSurfaceItem::mutex);

    if (m_item && m_item->surface()) {
        //Update if the item is dirty and we haven't done an updateTexture for this frame
        if (m_item->m_damaged && !m_textureUpdated) {
            m_item->updateTexture();
            updateTexture();
        }
    }
    //Reset value for next frame: we have not done updatePaintNode yet
    m_textureUpdated = false;
}

void QWaylandSurfaceNode::updateTexture()
{
    Q_ASSERT(m_item && m_item->textureProvider());
    QSGTexture *texture = m_item->textureProvider()->texture();
    setTexture(texture);
}

void QWaylandSurfaceNode::setItem(QWaylandSurfaceItem *item)
{
    m_item = item;
}

QT_END_NAMESPACE
