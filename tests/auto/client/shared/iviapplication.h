/****************************************************************************
**
** Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MOCKCOMPOSITOR_IVIAPPLICATION_H
#define MOCKCOMPOSITOR_IVIAPPLICATION_H

#include "coreprotocol.h"
#include <qwayland-server-ivi-application.h>

#include <QList>

namespace MockCompositor {

class Surface;
class IviApplication;
class IviSurface;

class IviApplication : public Global, public QtWaylandServer::ivi_application
{
    Q_OBJECT
public:
    explicit IviApplication(CoreCompositor *compositor);

    QList<IviSurface *> m_iviSurfaces;
protected:
    void ivi_application_surface_create(Resource *resource, uint32_t ivi_id, struct ::wl_resource *surface, uint32_t id) override;

};

class IviSurface : public QObject, public QtWaylandServer::ivi_surface
{
    Q_OBJECT
public:
    IviSurface(IviApplication *iviApplication, Surface *surface, uint32_t ivi_id, wl_client *client, int id, int version);

    void ivi_surface_destroy_resource(Resource *resource) override;
    void ivi_surface_destroy(Resource *resource) override;

    const uint m_iviId = 0;
private:
    IviApplication *m_iviApplication;

};


} // namespace MockCompositor

#endif // MOCKCOMPOSITOR_IVIAPPLICATION_H
