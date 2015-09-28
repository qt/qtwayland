/****************************************************************************
**
** Copyright (C) 2015 LG Electronics Inc, author: <mikko.levonmaa@lge.com>
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

#include "qwaylandtexturebufferattacher_p.h"

#include <QQuickWindow>
#include <QSGTexture>

#include <QtCompositor/qwaylandquicksurface.h>
#include <QtCompositor/qwaylandoutput.h>

QWaylandTextureBufferAttacher::QWaylandTextureBufferAttacher(QWaylandQuickSurface *surface)
    : m_surface(surface)
    , m_texture(0)
    , m_update(false)
{
}

QWaylandTextureBufferAttacher::~QWaylandTextureBufferAttacher()
{
    if (m_texture)
        m_texture->deleteLater();
    m_buffer = QWaylandBufferRef();
    m_nextBuffer = QWaylandBufferRef();
}

void QWaylandTextureBufferAttacher::attach(const QWaylandBufferRef &ref)
{
    m_nextBuffer = ref;
    m_update = true;
}

void QWaylandTextureBufferAttacher::updateTexture()
{
    m_buffer = m_nextBuffer;
    delete m_texture;
    m_texture = 0;

    QQuickWindow *window = static_cast<QQuickWindow *>(m_surface->mainOutput()->window());
    if (m_nextBuffer) {
        if (m_buffer.isShm()) {
            m_texture = window->createTextureFromImage(m_buffer.image());
        } else {
            QQuickWindow::CreateTextureOptions opt = 0;
            if (m_surface->useTextureAlpha()) {
                opt |= QQuickWindow::TextureHasAlphaChannel;
            }
            m_texture = window->createTextureFromId(m_buffer.createTexture(), m_surface->size(), opt);
        }
        m_texture->bind();
    }

    m_update = false;
}

void QWaylandTextureBufferAttacher::unmap()
{
    m_nextBuffer = QWaylandBufferRef();
    m_update = true;
}

void QWaylandTextureBufferAttacher::invalidateTexture()
{
    if (m_buffer)
        m_buffer.destroyTexture();
    delete m_texture;
    m_texture = 0;
    m_update = true;
    m_buffer = QWaylandBufferRef();
}

