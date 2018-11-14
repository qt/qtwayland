/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef QWAYLANDWLSCALER_H
#define QWAYLANDWLSCALER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

QT_BEGIN_NAMESPACE

#if QT_DEPRECATED_SINCE(5, 13)
class QWaylandWlScalerPrivate;

// TODO: We should have used the QT_DEPRECATED macro here, but for some reason
// header file generation stops working when that's added.
class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandWlScaler
        : public QWaylandCompositorExtensionTemplate<QWaylandWlScaler>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandWlScaler)

public:
    explicit QWaylandWlScaler();
    explicit QWaylandWlScaler(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
};
#endif

QT_END_NAMESPACE

#endif // QWAYLANDWLSCALER_H
