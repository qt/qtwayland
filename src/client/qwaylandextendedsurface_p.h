/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QWAYLANDEXTENDEDSURFACE_H
#define QWAYLANDEXTENDEDSURFACE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QString>
#include <QtCore/QVariant>

#include <QtWaylandClient/private/qwaylandclientexport_p.h>

#include <wayland-client.h>
#include <QtWaylandClient/private/qwayland-surface-extension.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandExtendedSurface : public QtWayland::qt_extended_surface
{
public:
    QWaylandExtendedSurface(QWaylandWindow *window);
    ~QWaylandExtendedSurface();

    void setContentOrientationMask(Qt::ScreenOrientations mask);

    void updateGenericProperty(const QString &name, const QVariant &value);

    Qt::WindowFlags setWindowFlags(Qt::WindowFlags flags);

private:
    void extended_surface_onscreen_visibility(int32_t visibility) Q_DECL_OVERRIDE;
    void extended_surface_set_generic_property(const QString &name, wl_array *value) Q_DECL_OVERRIDE;
    void extended_surface_close() Q_DECL_OVERRIDE;

    QWaylandWindow *m_window;
    QVariantMap m_properties;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDEXTENDEDSURFACE_H
