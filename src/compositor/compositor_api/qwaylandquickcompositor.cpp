/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#include <QtQml/QQmlEngine>

#include "qwaylandclient.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandquicksurface.h"
#include "qwaylandquickoutput.h"
#include "qwaylandquickitem.h"
#include "qwaylandoutput.h"

QT_BEGIN_NAMESPACE

QWaylandQuickCompositor::QWaylandQuickCompositor(QObject *parent)
    : QWaylandCompositor(parent)
    , m_initializeLegazyQmlNames(true)
{
}

void QWaylandQuickCompositor::create()
{
    if (m_initializeLegazyQmlNames)
        registerLegacyQmlNames();

    QWaylandCompositor::create();
}

void QWaylandQuickCompositor::registerLegacyQmlNames()
{
    static bool initialized = false;
    if (!initialized) {
        qmlRegisterUncreatableType<QWaylandQuickItem>("QtCompositor", 1, 0, "WaylandSurfaceItem", QObject::tr("Cannot create instance of WaylandSurfaceItem"));
        qmlRegisterUncreatableType<QWaylandQuickSurface>("QtCompositor", 1, 0, "WaylandQuickSurface", QObject::tr("Cannot create instance of WaylandQuickSurface"));
        qmlRegisterUncreatableType<QWaylandClient>("QtCompositor", 1, 0, "WaylandClient", QObject::tr("Cannot create instance of WaylandClient"));
        qmlRegisterUncreatableType<QWaylandOutput>("QtCompositor", 1, 0, "WaylandOutput", QObject::tr("Cannot create instance of WaylandOutput"));
        initialized = true;
    }
}

bool QWaylandQuickCompositor::initializeLegazyQmlNames() const
{
    return m_initializeLegazyQmlNames;
}

void QWaylandQuickCompositor::setInitializeLegazyQmlNames(bool init)
{
    if (isCreated())
        qWarning() << Q_FUNC_INFO << "modifying initializeLegazyQmlNames after the compositor is created is not supported";
    m_initializeLegazyQmlNames = init;
}

QWaylandOutput *QWaylandQuickCompositor::createOutput(QWaylandOutputSpace *outputSpace,
                                                      QWindow *window,
                                                      const QString &manufacturer,
                                                      const QString &model)
{
    QQmlEngine::setObjectOwnership(window, QQmlEngine::CppOwnership);

    QQuickWindow *quickWindow = qobject_cast<QQuickWindow *>(window);
    if (!quickWindow)
        qFatal("%s: couldn't cast QWindow to QQuickWindow. All output windows must "
               "be QQuickWindow derivates when using QWaylandQuickCompositor", Q_FUNC_INFO);
    QWaylandQuickOutput *output = new QWaylandQuickOutput(outputSpace, quickWindow, manufacturer, model);
    QQmlEngine::setObjectOwnership(output, QQmlEngine::CppOwnership);
    return output;
}

QWaylandSurface *QWaylandQuickCompositor::createSurface(QWaylandClient *client, quint32 id, int version)
{
    return new QWaylandQuickSurface(client, id, version, this);
}

void QWaylandQuickCompositor::classBegin()
{
}

void QWaylandQuickCompositor::componentComplete()
{
}

QT_END_NAMESPACE
