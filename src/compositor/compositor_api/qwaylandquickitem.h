// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSURFACEITEM_H
#define QWAYLANDSURFACEITEM_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

#include <QtQuick/QQuickItem>
#include <QtQuick/qsgtexture.h>

#include <QtQuick/qsgtextureprovider.h>

#include <QtWaylandCompositor/qwaylandview.h>
#include <QtWaylandCompositor/qwaylandquicksurface.h>

Q_DECLARE_METATYPE(QWaylandQuickSurface*)

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class QWaylandSeat;
class QWaylandQuickItemPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickItem : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickItem)
    Q_PROPERTY(QWaylandCompositor *compositor READ compositor NOTIFY compositorChanged)
    Q_PROPERTY(QWaylandSurface *surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(bool paintEnabled READ isPaintEnabled WRITE setPaintEnabled NOTIFY paintEnabledChanged)
    Q_PROPERTY(bool touchEventsEnabled READ touchEventsEnabled WRITE setTouchEventsEnabled NOTIFY touchEventsEnabledChanged)
    Q_PROPERTY(QWaylandSurface::Origin origin READ origin NOTIFY originChanged)
    Q_PROPERTY(bool inputEventsEnabled READ inputEventsEnabled WRITE setInputEventsEnabled NOTIFY inputEventsEnabledChanged)
    Q_PROPERTY(bool focusOnClick READ focusOnClick WRITE setFocusOnClick NOTIFY focusOnClickChanged)
    Q_PROPERTY(QObject *subsurfaceHandler READ subsurfaceHandler WRITE setSubsurfaceHandler NOTIFY subsurfaceHandlerChanged)
    Q_PROPERTY(QWaylandOutput *output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(bool bufferLocked READ isBufferLocked WRITE setBufferLocked NOTIFY bufferLockedChanged)
    Q_PROPERTY(bool allowDiscardFrontBuffer READ allowDiscardFrontBuffer WRITE setAllowDiscardFrontBuffer NOTIFY allowDiscardFrontBufferChanged)
    Q_MOC_INCLUDE("qwaylandcompositor.h")
    Q_MOC_INCLUDE("qwaylandseat.h")
    Q_MOC_INCLUDE("qwaylanddrag.h")
    QML_NAMED_ELEMENT(WaylandQuickItem)
    QML_ADDED_IN_VERSION(1, 0)
public:
    QWaylandQuickItem(QQuickItem *parent = nullptr);
    ~QWaylandQuickItem() override;

    QWaylandCompositor *compositor() const;
    QWaylandView *view() const;

    QWaylandSurface *surface() const;
    void setSurface(QWaylandSurface *surface);

    QWaylandSurface::Origin origin() const;

    bool isTextureProvider() const override;
    QSGTextureProvider *textureProvider() const override;

    bool isPaintEnabled() const;
    bool touchEventsEnabled() const;

    void setTouchEventsEnabled(bool enabled);

    bool inputEventsEnabled() const;
    void setInputEventsEnabled(bool enabled);

    bool focusOnClick() const;
    void setFocusOnClick(bool focus);

    bool inputRegionContains(const QPointF &localPosition) const;
    Q_INVOKABLE QPointF mapToSurface(const QPointF &point) const;
    Q_REVISION(1, 13) Q_INVOKABLE QPointF mapFromSurface(const QPointF &point) const;

#if QT_CONFIG(im)
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    Q_INVOKABLE QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;
#endif

    QObject *subsurfaceHandler() const;
    void setSubsurfaceHandler(QObject*);

    QWaylandOutput *output() const;
    void setOutput(QWaylandOutput *output);

    bool isBufferLocked() const;
    void setBufferLocked(bool locked);

    bool allowDiscardFrontBuffer() const;
    void setAllowDiscardFrontBuffer(bool discard);

    Q_INVOKABLE void setPrimary();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void touchEvent(QTouchEvent *event) override;
    void touchUngrabEvent() override;

#if QT_CONFIG(im)
    void inputMethodEvent(QInputMethodEvent *event) override;
#endif

    virtual void surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface);
public Q_SLOTS:
    virtual void takeFocus(QWaylandSeat *device = nullptr);
    void setPaintEnabled(bool paintEnabled);
    void raise();
    void lower();
    void sendMouseMoveEvent(const QPointF &position, QWaylandSeat *seat = nullptr);

private Q_SLOTS:
    void surfaceMappedChanged();
    void handleSurfaceChanged();
    void parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent);
    void updateSize();
    void updateBuffer(bool hasBuffer);
    void updateWindow();
    void updateOutput();
    void beforeSync();
    void handleSubsurfaceAdded(QWaylandSurface *childSurface);
    void handleSubsurfacePosition(const QPoint &pos);
    void handlePlaceAbove(QWaylandSurface *referenceSurface);
    void handlePlaceBelow(QWaylandSurface *referenceSurface);
#if QT_CONFIG(draganddrop)
    void handleDragStarted(QWaylandDrag *drag);
#endif
#if QT_CONFIG(im)
    void updateInputMethod(Qt::InputMethodQueries queries);
#endif
    void updateFocus();

Q_SIGNALS:
    void surfaceChanged();
    void compositorChanged();
    void paintEnabledChanged();
    void touchEventsEnabledChanged();
    void originChanged();
    void surfaceDestroyed();
    void inputEventsEnabledChanged();
    void focusOnClickChanged();
    void mouseMove(const QPointF &windowPosition);
    void mouseRelease();
    void subsurfaceHandlerChanged();
    void outputChanged();
    void bufferLockedChanged();
    void allowDiscardFrontBufferChanged();
protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

    QWaylandQuickItem(QWaylandQuickItemPrivate &dd, QQuickItem *parent = nullptr);
};

QT_END_NAMESPACE

#endif
