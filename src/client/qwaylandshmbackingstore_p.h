/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QWAYLANDSHMBACKINGSTORE_H
#define QWAYLANDSHMBACKINGSTORE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandClient/private/qwaylandbuffer_p.h>

#include <qpa/qplatformbackingstore.h>
#include <QtGui/QImage>
#include <qpa/qplatformwindow.h>
#include <QMutex>
#include <QLinkedList>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandAbstractDecoration;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShmBuffer : public QWaylandBuffer {
public:
    QWaylandShmBuffer(QWaylandDisplay *display,
           const QSize &size, QImage::Format format, int scale = 1);
    ~QWaylandShmBuffer();
    QSize size() const Q_DECL_OVERRIDE { return mImage.size(); }
    int scale() const Q_DECL_OVERRIDE { return int(mImage.devicePixelRatio()); }
    QImage *image() { return &mImage; }

    QImage *imageInsideMargins(const QMargins &margins);
private:
    QImage mImage;
    struct wl_shm_pool *mShmPool;
    QMargins mMargins;
    QImage *mMarginsImage;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandShmBackingStore : public QPlatformBackingStore
{
public:
    QWaylandShmBackingStore(QWindow *window);
    ~QWaylandShmBackingStore();

    QPaintDevice *paintDevice();
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) Q_DECL_OVERRIDE;
    void resize(const QSize &size, const QRegion &staticContents) Q_DECL_OVERRIDE;
    void resize(const QSize &size);
    void beginPaint(const QRegion &) Q_DECL_OVERRIDE;
    void endPaint() Q_DECL_OVERRIDE;
    void hidden();

    QWaylandAbstractDecoration *windowDecoration() const;

    QMargins windowDecorationMargins() const;
    QImage *entireSurface() const;
    QImage *contentSurface() const;
    void ensureSize();

    QWaylandWindow *waylandWindow() const;
    void iterateBuffer();

#ifndef QT_NO_OPENGL
    QImage toImage() const Q_DECL_OVERRIDE;
#endif

private:
    void updateDecorations();
    QWaylandShmBuffer *getBuffer(const QSize &size);

    QWaylandDisplay *mDisplay;
    QLinkedList<QWaylandShmBuffer *> mBuffers;
    QWaylandShmBuffer *mFrontBuffer;
    QWaylandShmBuffer *mBackBuffer;
    bool mPainting;
    QMutex mMutex;

    QSize mRequestedSize;
    Qt::WindowFlags mCurrentWindowFlags;
};

}

QT_END_NAMESPACE

#endif
