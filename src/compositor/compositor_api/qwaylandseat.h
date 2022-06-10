// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSEAT_H
#define QWAYLANDSEAT_H

#include <QtCore/qnamespace.h>
#include <QtCore/QPoint>
#include <QtCore/QString>

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>
#include <QtWaylandCompositor/qwaylandkeyboard.h>
#include <QtWaylandCompositor/qwaylandview.h>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandSurface;
class QKeyEvent;
class QTouchEvent;
class QInputEvent;
class QWaylandSeatPrivate;
class QWaylandDrag;
class QWaylandKeyboard;
class QWaylandPointer;
class QWaylandTouch;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandSeat : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandSeat)

#if QT_CONFIG(draganddrop)
    Q_PROPERTY(QWaylandDrag *drag READ drag CONSTANT)
    Q_MOC_INCLUDE("qwaylanddrag.h")
#endif
    Q_PROPERTY(QWaylandKeymap *keymap READ keymap CONSTANT)
    Q_MOC_INCLUDE("qwaylandkeymap.h")
    Q_MOC_INCLUDE("qwaylandview.h")

    QML_NAMED_ELEMENT(WaylandSeat)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    enum CapabilityFlag {
        // The order should match the enum WL_SEAT_CAPABILITY_*
        Pointer = 0x01,
        Keyboard = 0x02,
        Touch = 0x04,

        DefaultCapabilities = Pointer | Keyboard | Touch
    };
    Q_DECLARE_FLAGS(CapabilityFlags, CapabilityFlag)
    Q_ENUM(CapabilityFlags)

    QWaylandSeat(QWaylandCompositor *compositor, CapabilityFlags capabilityFlags = DefaultCapabilities);
    ~QWaylandSeat() override;
    virtual void initialize();
    bool isInitialized() const;

    void sendMousePressEvent(Qt::MouseButton button);
    void sendMouseReleaseEvent(Qt::MouseButton button);
    void sendMouseMoveEvent(QWaylandView *surface , const QPointF &localPos, const QPointF &outputSpacePos = QPointF());
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendFullKeyEvent(QKeyEvent *event);
    Q_INVOKABLE void sendKeyEvent(int qtKey, bool pressed);

    uint sendTouchPointEvent(QWaylandSurface *surface, int id, const QPointF &point, Qt::TouchPointState state);
    Q_INVOKABLE uint sendTouchPointPressed(QWaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE uint sendTouchPointReleased(QWaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE uint sendTouchPointMoved(QWaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE void sendTouchFrameEvent(QWaylandClient *client);
    Q_INVOKABLE void sendTouchCancelEvent(QWaylandClient *client);

    void sendFullTouchEvent(QWaylandSurface *surface, QTouchEvent *event);

    QWaylandPointer *pointer() const;
    //Normally set by the mouse device,
    //But can be set manually for use with touch or can reset unset the current mouse focus;
    QWaylandView *mouseFocus() const;
    void setMouseFocus(QWaylandView *view);

    QWaylandKeyboard *keyboard() const;
    QWaylandSurface *keyboardFocus() const;
    bool setKeyboardFocus(QWaylandSurface *surface);
    QWaylandKeymap *keymap();

    QWaylandTouch *touch() const;

    QWaylandCompositor *compositor() const;

#if QT_CONFIG(draganddrop)
    QWaylandDrag *drag() const;
#endif

    QWaylandSeat::CapabilityFlags capabilities() const;

    virtual bool isOwner(QInputEvent *inputEvent) const;

    static QWaylandSeat *fromSeatResource(struct ::wl_resource *resource);

Q_SIGNALS:
    void mouseFocusChanged(QWaylandView *newFocus, QWaylandView *oldFocus);
    void keyboardFocusChanged(QWaylandSurface *newFocus, QWaylandSurface *oldFocus);
#if QT_DEPRECATED_SINCE(6, 1)
    void cursorSurfaceRequest(QWaylandSurface *surface, int hotspotX, int hotspotY);
#endif
    void cursorSurfaceRequested(QWaylandSurface *surface, int hotspotX, int hotspotY, QWaylandClient *client);

private:
    void handleMouseFocusDestroyed();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWaylandSeat::CapabilityFlags)

QT_END_NAMESPACE

#endif // QWAYLANDSEAT_H
