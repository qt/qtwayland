/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

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

#include <QtWaylandClient/qtwaylandclientglobal.h>

#include <QtWaylandClient/private/qwayland-surface-extension.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandExtendedSurface : public QtWayland::qt_extended_surface
{
public:
    QWaylandExtendedSurface(QWaylandWindow *window);
    ~QWaylandExtendedSurface() override;

    void setContentOrientationMask(Qt::ScreenOrientations mask);

    void updateGenericProperty(const QString &name, const QVariant &value);

    Qt::WindowFlags setWindowFlags(Qt::WindowFlags flags);

private:
    void extended_surface_onscreen_visibility(int32_t visibility) override;
    void extended_surface_set_generic_property(const QString &name, wl_array *value) override;
    void extended_surface_close() override;

    QWaylandWindow *m_window = nullptr;
    QVariantMap m_properties;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDEXTENDEDSURFACE_H
