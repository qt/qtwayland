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


#include "qwaylandextension.h"
#include "qwaylandextension_p.h"

#include <wayland-server.h>

QT_BEGIN_NAMESPACE

QWaylandExtension::QWaylandExtension(QWaylandExtensionContainer *container, QObject *parent)
    : QObject(*new QWaylandExtensionPrivate(container), parent)
{
    container->addExtension(this);
}

QWaylandExtension::QWaylandExtension(QWaylandExtensionPrivate &dd, QObject *parent)
    : QObject(dd,parent)
{
    d_func()->extension_container->addExtension(this);
}

QWaylandExtension::~QWaylandExtension()
{
    Q_D(QWaylandExtension);
    d->extension_container->removeExtension(this);
}

QWaylandExtensionContainer::~QWaylandExtensionContainer()
{
    foreach (QWaylandExtension *extension, extension_vector) {
        delete extension;
    }
}

QWaylandExtension *QWaylandExtensionContainer::extension(const QByteArray &name)
{
    for (int i = 0; i < extension_vector.size(); i++) {
        if (extension_vector.at(i)->extensionInterface()->name == name)
            return extension_vector.at(i);
    }
    return Q_NULLPTR;
}

QWaylandExtension *QWaylandExtensionContainer::extension(const wl_interface *interface)
{
    for (int i = 0; i < extension_vector.size(); i++) {
        if (extension_vector.at(i)->extensionInterface() == interface)
            return extension_vector.at(i);
    }
    return Q_NULLPTR;
}

QVector<QWaylandExtension *> QWaylandExtensionContainer::extensions() const
{
    return extension_vector;
}

void QWaylandExtensionContainer::addExtension(QWaylandExtension *extension)
{
    Q_ASSERT(!extension_vector.contains(extension));
    extension_vector.append(extension);
}

void QWaylandExtensionContainer::removeExtension(QWaylandExtension *extension)
{
    extension_vector.removeOne(extension);
}

QT_END_NAMESPACE
