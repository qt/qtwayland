/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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
****************************************************************************/

#ifndef QWAYLANDPOINTER_H
#define QWAYLANDPOINTER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandPointer;
class QWaylandPointerPrivate;
class QWaylandSeat;
class QWaylandView;
class QWaylandOutput;
class QWaylandClient;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandPointer : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandPointer)
    Q_PROPERTY(bool isButtonPressed READ isButtonPressed NOTIFY buttonPressedChanged)
public:
    QWaylandPointer(QWaylandSeat *seat, QObject *parent = nullptr);

    QWaylandSeat *seat() const;
    QWaylandCompositor *compositor() const;

    QWaylandOutput *output() const;
    void setOutput(QWaylandOutput *output);

    virtual uint sendMousePressEvent(Qt::MouseButton button);
    virtual uint sendMouseReleaseEvent(Qt::MouseButton button);
    virtual void sendMouseMoveEvent(QWaylandView *view, const QPointF &localPos, const QPointF &outputSpacePos);
    virtual void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    QWaylandView *mouseFocus() const;
    QPointF currentLocalPosition() const;
    QPointF currentSpacePosition() const;

    bool isButtonPressed() const;

    virtual void addClient(QWaylandClient *client, uint32_t id, uint32_t version);

    wl_resource *focusResource() const;

    static uint32_t toWaylandButton(Qt::MouseButton button);
    uint sendButton(struct wl_resource *resource, uint32_t time, Qt::MouseButton button, uint32_t state);
Q_SIGNALS:
    void outputChanged();
    void buttonPressedChanged();

private:
    void enteredSurfaceDestroyed(void *data);
    void pointerFocusChanged(QWaylandView *newFocus, QWaylandView *oldFocus);
};

QT_END_NAMESPACE

#endif  /*QWAYLANDPOINTER_H*/
