// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
#include <QtCore/QMap> // for QVariantMap

#include <QtWaylandClient/qtwaylandclientglobal.h>

#include <QtWaylandClient/private/qwayland-surface-extension.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLANDCLIENT_EXPORT QWaylandExtendedSurface : public QtWayland::qt_extended_surface
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
