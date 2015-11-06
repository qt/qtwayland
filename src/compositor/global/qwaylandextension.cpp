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


#include "qwaylandextension.h"
#include "qwaylandextension_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <wayland-server.h>

QT_BEGIN_NAMESPACE

QWaylandExtension::QWaylandExtension()
    : QWaylandObject(*new QWaylandExtensionPrivate())
{
}

QWaylandExtension::QWaylandExtension(QWaylandObject *container)
    : QWaylandObject(*new QWaylandExtensionPrivate())
{
    d_func()->extension_container = container;
    QCoreApplication::postEvent(this, new QEvent(QEvent::Polish));
}

QWaylandExtension::QWaylandExtension(QWaylandExtensionPrivate &dd)
    : QWaylandObject(dd)
{
}

QWaylandExtension::QWaylandExtension(QWaylandObject *container, QWaylandExtensionPrivate &dd)
    : QWaylandObject(dd)
{
    d_func()->extension_container = container;
    QCoreApplication::postEvent(this, new QEvent(QEvent::Polish));
}

QWaylandExtension::~QWaylandExtension()
{
    Q_D(QWaylandExtension);
    if (d->extension_container)
        d->extension_container->removeExtension(this);
}

QWaylandObject *QWaylandExtension::extensionContainer() const
{
    Q_D(const QWaylandExtension);
    return d->extension_container;
}

void QWaylandExtension::setExtensionContainer(QWaylandObject *container)
{
    Q_D(QWaylandExtension);
    d->extension_container = container;
}

void QWaylandExtension::initialize()
{
    Q_D(QWaylandExtension);
    if (d->initialized) {
        qWarning() << "QWaylandExtension:" << extensionInterface()->name << "is already initialized";
        return;
    }

    if (!d->extension_container) {
        qWarning() << "QWaylandExtension:" << extensionInterface()->name << "requests to initialize with no extension container set";
        return;
    }

    d->extension_container->addExtension(this);
    d->initialized = true;
}

bool QWaylandExtension::isInitialized() const
{
    Q_D(const QWaylandExtension);
    return d->initialized;
}

bool QWaylandExtension::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::Polish:
        initialize();
        break;
    default:
        break;
    }
    return QWaylandObject::event(event);
}

QWaylandObject::QWaylandObject(QObject *parent)
    :QObject(parent)
{
}

QWaylandObject::QWaylandObject(QObjectPrivate &d, QObject *parent)
    :QObject(d, parent)
{
}


QWaylandObject::~QWaylandObject()
{
    foreach (QWaylandExtension *extension, extension_vector)
        QWaylandExtensionPrivate::get(extension)->extension_container = Q_NULLPTR;
}

QWaylandExtension *QWaylandObject::extension(const QByteArray &name)
{
    for (int i = 0; i < extension_vector.size(); i++) {
        if (extension_vector.at(i)->extensionInterface()->name == name)
            return extension_vector.at(i);
    }
    return Q_NULLPTR;
}

QWaylandExtension *QWaylandObject::extension(const wl_interface *interface)
{
    for (int i = 0; i < extension_vector.size(); i++) {
        if (extension_vector.at(i)->extensionInterface() == interface)
            return extension_vector.at(i);
    }
    return Q_NULLPTR;
}

QList<QWaylandExtension *> QWaylandObject::extensions() const
{
    return extension_vector;
}

void QWaylandObject::addExtension(QWaylandExtension *extension)
{
    Q_ASSERT(!extension_vector.contains(extension));
    extension_vector.append(extension);
}

void QWaylandObject::removeExtension(QWaylandExtension *extension)
{
    Q_ASSERT(extension_vector.contains(extension));
    extension_vector.removeOne(extension);
}

QT_END_NAMESPACE
