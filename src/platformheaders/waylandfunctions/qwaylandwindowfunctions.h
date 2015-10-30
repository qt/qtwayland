/****************************************************************************
**
** Copyright (C) 2015 LG Electronics Ltd, author: mikko.levonmaa@lge.com
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
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
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDWINDOWFUNCTIONS_H
#define QWAYLANDWINDOWFUNCTIONS_H

#include <QtCore/QByteArray>
#include <QtGui/QGuiApplication>

QT_BEGIN_NAMESPACE

class QWindow;

class QWaylandWindowFunctions {
public:

    typedef void (*SetWindowSync)(QWindow *window);
    typedef void (*SetWindowDeSync)(QWindow *window);
    typedef bool (*IsWindowSync)(QWindow *window);
    static const QByteArray setSyncIdentifier() { return QByteArrayLiteral("WaylandSubSurfaceSetSync"); }
    static const QByteArray setDeSyncIdentifier() { return QByteArrayLiteral("WaylandSubSurfaceSetDeSync"); }
    static const QByteArray isSyncIdentifier() { return QByteArrayLiteral("WaylandSubSurfaceIsSync"); }

    static void setSync(QWindow *window)
    {
        static SetWindowSync func = reinterpret_cast<SetWindowSync>(QGuiApplication::platformFunction(setSyncIdentifier()));
        Q_ASSERT(func);
        func(window);
    }

    static void setDeSync(QWindow *window)
    {
        static SetWindowDeSync func = reinterpret_cast<SetWindowDeSync>(QGuiApplication::platformFunction(setDeSyncIdentifier()));
        Q_ASSERT(func);
        func(window);
    }

    static bool isSync(QWindow *window)
    {
        static IsWindowSync func = reinterpret_cast<IsWindowSync>(QGuiApplication::platformFunction(isSyncIdentifier()));
        Q_ASSERT(func);
        return func(window);
    }

};

QT_END_NAMESPACE

#endif // QWAYLANDWINDOWFUNCTIONS_H

