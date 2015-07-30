/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "qwaylandinput.h"

#include "qwlinputdevice_p.h"
#include "qwlkeyboard_p.h"
#include "qwaylandcompositor.h"
#include "qwlsurface_p.h"
#include "qwlcompositor_p.h"
#include "qwaylandsurfaceview.h"

QT_BEGIN_NAMESPACE

QWaylandKeymap::QWaylandKeymap(const QString &layout, const QString &variant, const QString &options, const QString &model, const QString &rules)
              : m_layout(layout)
              , m_variant(variant)
              , m_options(options)
              , m_rules(rules)
              , m_model(model)
{
}



QWaylandInputDevice::QWaylandInputDevice(QWaylandCompositor *compositor, CapabilityFlags caps)
    : QObject(*new QWaylandInputDevicePrivate(this,compositor, caps))
{
}

QWaylandInputDevice::~QWaylandInputDevice()
{
}

void QWaylandInputDevice::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(QWaylandInputDevice);
    d->sendMousePressEvent(button);
}

void QWaylandInputDevice::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(QWaylandInputDevice);
    d->sendMouseReleaseEvent(button);
}

/** Convenience function that will set the mouse focus to the surface, then send the mouse move event.
 *  If the mouse focus is the same surface as the surface passed in, then only the move event is sent
 **/
void QWaylandInputDevice::sendMouseMoveEvent(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    Q_D(QWaylandInputDevice);
    d->sendMouseMoveEvent(surface,localPos,globalPos);
}

void QWaylandInputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(QWaylandInputDevice);
    d->sendMouseWheelEvent(orientation, delta);
}

void QWaylandInputDevice::sendResetCurrentMouseView()
{
    Q_D(QWaylandInputDevice);
    d->sendResetCurrentMouseView();
}

void QWaylandInputDevice::sendKeyPressEvent(uint code)
{
    keyboard()->sendKeyPressEvent(code);
}

void QWaylandInputDevice::sendKeyReleaseEvent(uint code)
{
    keyboard()->sendKeyReleaseEvent(code);
}

void QWaylandInputDevice::sendTouchPointEvent(int id, const QPointF &point, Qt::TouchPointState state)
{
    Q_D(QWaylandInputDevice);
    d->sendTouchPointEvent(id, point, state);
}

void QWaylandInputDevice::sendTouchFrameEvent()
{
    Q_D(QWaylandInputDevice);
    d->sendTouchFrameEvent();
}

void QWaylandInputDevice::sendTouchCancelEvent()
{
    Q_D(QWaylandInputDevice);
    d->sendTouchCancelEvent();
}

void QWaylandInputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    Q_D(QWaylandInputDevice);
    d->sendFullTouchEvent(event);
}

void QWaylandInputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    Q_D(QWaylandInputDevice);
    d->sendFullKeyEvent(event);
}

void QWaylandInputDevice::sendFullKeyEvent(QWaylandSurface *surface, QKeyEvent *event)
{
    Q_D(QWaylandInputDevice);
    d->sendFullKeyEvent(surface, event);
}

QWaylandKeyboard *QWaylandInputDevice::keyboard() const
{
    Q_D(const QWaylandInputDevice);
    return d->keyboardDevice();
}

QWaylandSurface *QWaylandInputDevice::keyboardFocus() const
{
    Q_D(const QWaylandInputDevice);
    QWaylandSurface *surface = d->keyboardFocus();
    return surface;
}

bool QWaylandInputDevice::setKeyboardFocus(QWaylandSurface *surface)
{
    Q_D(QWaylandInputDevice);
    return d->setKeyboardFocus(surface);
}

void QWaylandInputDevice::setKeymap(const QWaylandKeymap &keymap)
{
    if (keyboard())
        keyboard()->setKeymap(keymap);
}

QWaylandSurfaceView *QWaylandInputDevice::mouseFocus() const
{
    Q_D(const QWaylandInputDevice);
    return d->mouseFocus();
}

QWaylandOutputSpace *QWaylandInputDevice::outputSpace() const
{
    Q_D(const QWaylandInputDevice);
    return d->outputSpace();
}

void QWaylandInputDevice::setOutputSpace(QWaylandOutputSpace *outputSpace)
{
    Q_D(QWaylandInputDevice);
    d->setOutputSpace(outputSpace);
}

QWaylandCompositor *QWaylandInputDevice::compositor() const
{
    Q_D(const QWaylandInputDevice);
    return d->compositor();
}

QWaylandDrag *QWaylandInputDevice::drag() const
{
    Q_D(const QWaylandInputDevice);
    return d->dragHandle();
}

QWaylandInputDevice::CapabilityFlags QWaylandInputDevice::capabilities() const
{
    Q_D(const QWaylandInputDevice);
    return d->capabilities();
}

bool QWaylandInputDevice::isOwner(QInputEvent *inputEvent) const
{
    Q_UNUSED(inputEvent);
    return true;
}

QT_END_NAMESPACE
