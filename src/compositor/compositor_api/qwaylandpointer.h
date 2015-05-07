/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDPOINTER_H
#define QWAYLANDPOINTER_H

#include <QtCompositor/QWaylandExtensionContainer>

QT_BEGIN_NAMESPACE

class QWaylandPointer;
class QWaylandPointerPrivate;
class QWaylandInputDevice;
class QWaylandSurfaceView;
class QWaylandOutput;
class QWaylandClient;

class Q_COMPOSITOR_EXPORT QWaylandPointerGrabber
{
public:
    QWaylandPointerGrabber()
        : pointer(Q_NULLPTR)
    {}
    virtual ~QWaylandPointerGrabber();

    virtual void focus() = 0;
    virtual void motion(uint32_t time) = 0;
    virtual void button(uint32_t time, Qt::MouseButton button, uint32_t state) = 0;

    QWaylandPointer *pointer;
};

class Q_COMPOSITOR_EXPORT QWaylandDefaultPointerGrabber : public QWaylandPointerGrabber
{
public:
    QWaylandDefaultPointerGrabber()
        : QWaylandPointerGrabber()
    {}
    QWaylandDefaultPointerGrabber(QWaylandPointer *pointer)
        : QWaylandPointerGrabber()
    { this->pointer = pointer; }

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;
};
class Q_COMPOSITOR_EXPORT QWaylandPointer : public QObject, public QWaylandExtensionContainer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandPointer)
    Q_PROPERTY(bool isButtonPressed READ isButtonPressed NOTIFY buttonPressedChanged)
public:
    QWaylandPointer(QWaylandInputDevice *seat, QObject *parent = 0);

    QWaylandInputDevice *inputDevice() const;
    QWaylandCompositor *compositor() const;

    QWaylandOutput *output() const;
    void setOutput(QWaylandOutput *output);

    void startGrab(QWaylandPointerGrabber *currentGrab);
    void endGrab();
    QWaylandPointerGrabber *currentGrab() const;
    Qt::MouseButton grabButton() const;

    uint32_t grabTime() const;
    uint32_t grabSerial() const;

    virtual void sendMousePressEvent(Qt::MouseButton button);
    virtual void sendMouseReleaseEvent(Qt::MouseButton button);
    virtual void sendMouseMoveEvent(QWaylandSurfaceView *view, const QPointF &localPos, const QPointF &outputSpacePos);
    virtual void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    QWaylandSurfaceView *currentView() const;
    QPointF currentLocalPosition() const;
    QPointF currentSpacePosition() const;

    bool isButtonPressed() const;

    void addClient(QWaylandClient *client, uint32_t id);

    struct wl_resource *focusResource() const;

    static uint32_t toWaylandButton(Qt::MouseButton button);
    void sendButton(struct wl_resource *resource, uint32_t time, Qt::MouseButton button, uint32_t state);
Q_SIGNALS:
    void outputChanged();
    void buttonPressedChanged();

private:
    void focusDestroyed(void *data);
    void pointerFocusChanged(QWaylandSurfaceView *newFocus, QWaylandSurfaceView *oldFocus);
};

QT_END_NAMESPACE

#endif  /*QWAYLANDPOINTER_H*/
