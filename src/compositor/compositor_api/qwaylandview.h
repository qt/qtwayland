// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSURFACEVIEW_H
#define QWAYLANDSURFACEVIEW_H

#include <QtWaylandCompositor/QWaylandBufferRef>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

#include <QtCore/QPointF>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandViewPrivate;
class QWaylandOutput;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandView : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandView)
    Q_PROPERTY(QObject *renderObject READ renderObject CONSTANT)
    Q_PROPERTY(QWaylandSurface *surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandOutput *output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(bool bufferLocked READ isBufferLocked WRITE setBufferLocked NOTIFY bufferLockedChanged)
    Q_PROPERTY(bool allowDiscardFrontBuffer READ allowDiscardFrontBuffer WRITE setAllowDiscardFrontBuffer NOTIFY allowDiscardFrontBufferChanged)
    Q_MOC_INCLUDE("qwaylandoutput.h")
public:
    QWaylandView(QObject *renderObject = nullptr, QObject *parent = nullptr);
    ~QWaylandView() override;

    QObject *renderObject() const;

    QWaylandSurface *surface() const;
    void setSurface(QWaylandSurface *surface);

    QWaylandOutput *output() const;
    void setOutput(QWaylandOutput *output);

    virtual void bufferCommitted(const QWaylandBufferRef &buffer, const QRegion &damage);
    virtual bool advance();
    virtual void discardCurrentBuffer();
    virtual QWaylandBufferRef currentBuffer();
    virtual QRegion currentDamage();

    bool isBufferLocked() const;
    void setBufferLocked(bool locked);

    bool allowDiscardFrontBuffer() const;
    void setAllowDiscardFrontBuffer(bool discard);

    void setPrimary();
    bool isPrimary() const;

    struct wl_resource *surfaceResource() const;

Q_SIGNALS:
    void surfaceChanged();
    void surfaceDestroyed();
    void outputChanged();
    void bufferLockedChanged();
    void allowDiscardFrontBufferChanged();
};

QT_END_NAMESPACE

#endif
