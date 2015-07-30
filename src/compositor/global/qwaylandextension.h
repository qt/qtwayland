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

#ifndef QWAYLANDEXTENSION_H
#define QWAYLANDEXTENSION_H

#include <QtCompositor/qwaylandexport.h>
#include <wayland-server.h>

#include <QtCore/QObject>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandExtensionContainer;
class QWaylandExtensionPrivate;
class QWaylandExtensionTemplatePrivate;

class Q_COMPOSITOR_EXPORT QWaylandExtension : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandExtension)
public:
    QWaylandExtension(QWaylandExtensionContainer *container, QObject *parent = 0);
    virtual ~QWaylandExtension();

    virtual const struct wl_interface *interface() const = 0;

    const QByteArray name() const { return interface()->name; }
    quint32 version() const { return interface()->version; }

protected:
    QWaylandExtension(QWaylandExtensionPrivate &dd, QObject *parent = 0);
};

class Q_COMPOSITOR_EXPORT QWaylandExtensionContainer
{
public:
    virtual ~QWaylandExtensionContainer();

    QWaylandExtension *extension(const QByteArray &name);
    QWaylandExtension *extension(const wl_interface *interface);
    QVector<QWaylandExtension *> extensions() const;
    void addExtension(QWaylandExtension *extension);
    void removeExtension(QWaylandExtension *extension);

protected:
    QVector<QWaylandExtension *> extension_vector;
};

class Q_COMPOSITOR_EXPORT QWaylandExtensionTemplate : public QWaylandExtension
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandExtensionTemplate)
public:
    QWaylandExtensionTemplate(QWaylandExtensionContainer *container, QObject *parent = 0);

    const struct wl_interface *interface() const Q_DECL_OVERRIDE;

protected:
    QWaylandExtensionTemplate(QWaylandExtensionTemplatePrivate &dd, QObject *parent = 0);
};

QT_END_NAMESPACE

#endif
