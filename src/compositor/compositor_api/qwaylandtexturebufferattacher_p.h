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

#ifndef QWAYLANDTEXTUREBUFFERATTACHER_H
#define QWAYLANDTEXTUREBUFFERATTACHER_H

#include <QtCompositor/qwaylandsurface.h>
#include <QtCompositor/qwaylandbufferref.h>

QT_BEGIN_NAMESPACE

class QSGTexture;
class QWaylandQuickSurface;

class QWaylandTextureBufferAttacher : public QWaylandBufferAttacher
{
public:
    QWaylandTextureBufferAttacher(QWaylandQuickSurface *surface);
    virtual ~QWaylandTextureBufferAttacher();

    void updateTexture();
    void invalidateTexture();

    bool isDirty() const { return m_update; }
    QSGTexture* texture() const { return m_texture; }
    QWaylandBufferRef currentBuffer() const { return m_buffer; }

protected:
    virtual void attach(const QWaylandBufferRef &ref) Q_DECL_OVERRIDE;
    virtual void unmap() Q_DECL_OVERRIDE;

    QWaylandQuickSurface *m_surface;
    QSGTexture *m_texture;

    QWaylandBufferRef m_buffer;
    QWaylandBufferRef m_nextBuffer;
    bool m_update;
};

QT_END_NAMESPACE
#endif


