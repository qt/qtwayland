/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidi ary(-ies).
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#include "qwaylandkeyboard.h"
#include "qwlkeyboard_p.h"
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/QWaylandClient>

QT_BEGIN_NAMESPACE

QWaylandKeyboardGrabber::~QWaylandKeyboardGrabber()
{
}

QWaylandKeyboard::QWaylandKeyboard(QWaylandInputDevice *seat, QObject *parent)
    : QObject(* new QWaylandKeyboardPrivate(seat), parent)
{
    Q_D(QWaylandKeyboard);
    connect(&d->m_focusDestroyListener, &QWaylandDestroyListener::fired, this, &QWaylandKeyboard::focusDestroyed);
}

QWaylandInputDevice *QWaylandKeyboard::inputDevice() const
{
    Q_D(const QWaylandKeyboard);
    return d->m_seat;
}

QWaylandCompositor *QWaylandKeyboard::compositor() const
{
    Q_D(const QWaylandKeyboard);
    return d->m_seat->compositor();
}

void QWaylandKeyboard::focusDestroyed(void *data)
{
    Q_UNUSED(data);
    Q_D(QWaylandKeyboard);
    d->m_focusDestroyListener.reset();

    d->m_focus = 0;
    d->m_focusResource = 0;
}

QWaylandClient *QWaylandKeyboard::focusClient() const
{
    Q_D(const QWaylandKeyboard);
    if (!d->focusResource())
        return Q_NULLPTR;
    return QWaylandClient::fromWlClient(compositor(), d->focusResource()->client());
}

void QWaylandKeyboard::sendKeyModifiers(QWaylandClient *client, uint serial)
{
    Q_D(QWaylandKeyboard);
    QtWaylandServer::wl_keyboard::Resource *resource = d->resourceMap().value(client->client());
    if (resource)
        d->sendKeyModifiers(resource, serial);
}
void QWaylandKeyboard::sendKeyPressEvent(uint code)
{
    Q_D(QWaylandKeyboard);
    d->sendKeyPressEvent(code);
}

void QWaylandKeyboard::sendKeyReleaseEvent(uint code)
{
    Q_D(QWaylandKeyboard);
    d->sendKeyReleaseEvent(code);
}

QWaylandSurface *QWaylandKeyboard::focus() const
{
    Q_D(const QWaylandKeyboard);
    return d->focus();
}

bool QWaylandKeyboard::setFocus(QWaylandSurface *surface)
{
    Q_D(QWaylandKeyboard);
    return d->setFocus(surface);
}

void QWaylandKeyboard::setKeymap(const QWaylandKeymap &keymap)
{
    Q_D(QWaylandKeyboard);
    d->setKeymap(keymap);
}

void QWaylandKeyboard::startGrab(QWaylandKeyboardGrabber *grab)
{
    Q_D(QWaylandKeyboard);
    d->startGrab(grab);
}

void QWaylandKeyboard::endGrab()
{
    Q_D(QWaylandKeyboard);
    d->endGrab();
}

QWaylandKeyboardGrabber *QWaylandKeyboard::currentGrab() const
{
    Q_D(const QWaylandKeyboard);
    return d->currentGrab();
}

void QWaylandKeyboard::addClient(QWaylandClient *client, uint32_t id)
{
    Q_D(QWaylandKeyboard);
    d->add(client->client(), id, QtWaylandServer::wl_keyboard::interfaceVersion());
}

QT_END_NAMESPACE
