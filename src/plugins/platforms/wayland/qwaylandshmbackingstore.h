/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDSHMBACKINGSTORE_H
#define QWAYLANDSHMBACKINGSTORE_H

#include "qwaylandbuffer.h"

#include "qwaylanddecoration.h"
#include "qwaylandwindow.h"

#include <qpa/qplatformbackingstore.h>
#include <QtGui/QImage>
#include <qpa/qplatformwindow.h>

QT_BEGIN_NAMESPACE

class QWaylandDisplay;

class QWaylandShmBuffer : public QWaylandBuffer {
public:
    QWaylandShmBuffer(QWaylandDisplay *display,
           const QSize &size, QImage::Format format);
    ~QWaylandShmBuffer();
    QSize size() const { return mImage.size(); }
    QImage *image() { return &mImage; }

    QImage *imageInsideMargins(const QMargins &margins);
private:
    QImage mImage;
    struct wl_shm_pool *mShmPool;
    QMargins mMargins;
    QImage *mMarginsImage;
};

class QWaylandShmBackingStore : public QPlatformBackingStore
{
public:
    QWaylandShmBackingStore(QWindow *window);
    ~QWaylandShmBackingStore();

    QPaintDevice *paintDevice();
    void flush(QWindow *window, const QRegion &region, const QPoint &offset);
    void resize(const QSize &size, const QRegion &staticContents);
    void resize(const QSize &size);
    void beginPaint(const QRegion &);
    void endPaint();

    QMargins windowDecorationMargins() const;
    QImage *entireSurface() const;
    void ensureSize();

    QWaylandWindow *waylandWindow() const;
    void iterateBuffer();

private:
    QWaylandDisplay *mDisplay;
    QWaylandShmBuffer *mFrontBuffer;
    QWaylandShmBuffer *mBackBuffer;
    bool mFrontBufferIsDirty;
    bool mPainting;

    QWaylandDecoration *mWindowDecoration;
    QSize mRequestedSize;
    Qt::WindowFlags mCurrentWindowFlags;

    static const struct wl_callback_listener frameCallbackListener;
    static void done(void *data,
             struct wl_callback *callback,
             uint32_t time);
    struct wl_callback *mFrameCallback;
};

inline QMargins QWaylandShmBackingStore::windowDecorationMargins() const
{
    if (mWindowDecoration)
        return mWindowDecoration->margins();
    return QMargins();
}

inline QWaylandWindow *QWaylandShmBackingStore::waylandWindow() const
{
    return static_cast<QWaylandWindow *>(window()->handle());
}

QT_END_NAMESPACE

#endif
