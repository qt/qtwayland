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
    : d(new QtWayland::InputDevice(this,compositor->handle(), caps))
{
}

QWaylandInputDevice::~QWaylandInputDevice()
{
    delete d;
}

void QWaylandInputDevice::sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMousePressEvent(button,localPos,globalPos);
}

void QWaylandInputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseReleaseEvent(button,localPos,globalPos);
}

void QWaylandInputDevice::sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseMoveEvent(localPos,globalPos);
}

/** Convenience function that will set the mouse focus to the surface, then send the mouse move event.
 *  If the mouse focus is the same surface as the surface passed in, then only the move event is sent
 **/
void QWaylandInputDevice::sendMouseMoveEvent(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseMoveEvent(surface,localPos,globalPos);
}

void QWaylandInputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    d->sendMouseWheelEvent(orientation, delta);
}

void QWaylandInputDevice::sendKeyPressEvent(uint code)
{
    d->keyboardDevice()->sendKeyPressEvent(code);
}

void QWaylandInputDevice::sendKeyReleaseEvent(uint code)
{
    d->keyboardDevice()->sendKeyReleaseEvent(code);
}

void QWaylandInputDevice::sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state)
{
    d->sendTouchPointEvent(id,x,y,state);
}

void QWaylandInputDevice::sendTouchFrameEvent()
{
    d->sendTouchFrameEvent();
}

void QWaylandInputDevice::sendTouchCancelEvent()
{
    d->sendTouchCancelEvent();
}

void QWaylandInputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    d->sendFullTouchEvent(event);
}

void QWaylandInputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    d->sendFullKeyEvent(event);
}

void QWaylandInputDevice::sendFullKeyEvent(QWaylandSurface *surface, QKeyEvent *event)
{
    d->sendFullKeyEvent(surface->handle(), event);
}

QWaylandSurface *QWaylandInputDevice::keyboardFocus() const
{
    QtWayland::Surface *wlsurface = d->keyboardFocus();
    if (wlsurface)
        return  wlsurface->waylandSurface();
    return 0;
}

bool QWaylandInputDevice::setKeyboardFocus(QWaylandSurface *surface)
{
    QtWayland::Surface *wlsurface = surface?surface->handle():0;
    return d->setKeyboardFocus(wlsurface);
}

void QWaylandInputDevice::setKeymap(const QWaylandKeymap &keymap)
{
    if (handle()->keyboardDevice())
        handle()->keyboardDevice()->setKeymap(keymap);
}

QWaylandSurfaceView *QWaylandInputDevice::mouseFocus() const
{
    return d->mouseFocus();
}

void QWaylandInputDevice::setMouseFocus(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos)
{
    d->setMouseFocus(surface,localPos,globalPos);
}

QWaylandCompositor *QWaylandInputDevice::compositor() const
{
    return d->compositor()->waylandCompositor();
}

QtWayland::InputDevice *QWaylandInputDevice::handle() const
{
    return d;
}

QWaylandInputDevice::CapabilityFlags QWaylandInputDevice::capabilities()
{
    return d->capabilities();
}

bool QWaylandInputDevice::isOwner(QInputEvent *inputEvent)
{
    Q_UNUSED(inputEvent);
    return true;
}

QT_END_NAMESPACE
