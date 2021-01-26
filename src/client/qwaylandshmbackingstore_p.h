/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
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

#include <list>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandAbstractDecoration;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShmBuffer : public QWaylandBuffer {
public:
    QWaylandShmBuffer(QWaylandDisplay *display,
           const QSize &size, QImage::Format format, int scale = 1);
    ~QWaylandShmBuffer() override;
    QSize size() const override { return mImage.size(); }
    int scale() const override { return int(mImage.devicePixelRatio()); }
    QImage *image() { return &mImage; }

    QImage *imageInsideMargins(const QMargins &margins);
private:
    QImage mImage;
    struct wl_shm_pool *mShmPool = nullptr;
    QMargins mMargins;
    QImage *mMarginsImage = nullptr;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandShmBackingStore : public QPlatformBackingStore
{
public:
    QWaylandShmBackingStore(QWindow *window, QWaylandDisplay *display);
    ~QWaylandShmBackingStore() override;

    QPaintDevice *paintDevice() override;
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;
    void resize(const QSize &size, const QRegion &staticContents) override;
    void resize(const QSize &size);
    void beginPaint(const QRegion &region) override;
    void endPaint() override;

    QWaylandAbstractDecoration *windowDecoration() const;

    QMargins windowDecorationMargins() const;
    QImage *entireSurface() const;
    QImage *contentSurface() const;
    void ensureSize();

    QWaylandWindow *waylandWindow() const;
    void iterateBuffer();

#if QT_CONFIG(opengl)
    QImage toImage() const override;
#endif

private:
    void updateDecorations();
    QWaylandShmBuffer *getBuffer(const QSize &size);

    QWaylandDisplay *mDisplay = nullptr;
    std::list<QWaylandShmBuffer *> mBuffers;
    QWaylandShmBuffer *mFrontBuffer = nullptr;
    QWaylandShmBuffer *mBackBuffer = nullptr;
    bool mPainting = false;
    bool mPendingFlush = false;
    QRegion mPendingRegion;
    QMutex mMutex;

    QSize mRequestedSize;
    Qt::WindowFlags mCurrentWindowFlags;
};

}

QT_END_NAMESPACE

#endif
