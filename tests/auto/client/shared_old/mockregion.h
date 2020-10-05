/****************************************************************************
**
** Copyright (C) 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef MOCKREGION_H
#define MOCKREGION_H

#include <qglobal.h>
#include <QSharedPointer>

#include "qwayland-server-wayland.h"

class MockRegion;

namespace Impl {

class Compositor;

class Region : public QtWaylandServer::wl_region
{
public:
    Region(wl_client *client, uint32_t id, int v, Compositor *compositor);
    ~Region();

    Compositor *compositor() const { return m_compositor; }
    static Region *fromResource(struct ::wl_resource *resource);

protected:
    void region_destroy_resource(Resource *resource) override;

private:
    Compositor *m_compositor = nullptr;
    QSharedPointer<MockRegion> m_mockRegion;
};

}

#endif // MOCKREGION_H
