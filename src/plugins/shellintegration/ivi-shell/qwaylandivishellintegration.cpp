// Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandivishellintegration.h"

#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>

#include "qwaylandivisurface_p.h"

#include <mutex>

#include <unistd.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandIviController : public QWaylandClientExtensionTemplate<QWaylandIviController>,
                              public QtWayland::ivi_controller
{
public:
    QWaylandIviController() : QWaylandClientExtensionTemplate(1) { }
    void initialize() { QWaylandClientExtensionTemplate::initialize(); }
};

QWaylandIviShellIntegration::QWaylandIviShellIntegration()
    : QWaylandShellIntegrationTemplate(1), m_iviController(new QWaylandIviController)
{
}

bool QWaylandIviShellIntegration::initialize(QWaylandDisplay *display)
{
    QWaylandShellIntegrationTemplate::initialize(display);
    m_iviController->initialize();
    return isActive();
}

/* get unique id
 * pattern1:
 *   When set QT_IVI_SURFACE_ID, We use it as ID.
 *   Next ID is increment.
 * pattern2:
 *   When not set QT_IVI_SURFACE_ID, We use process ID and unused bit.
 *   process ID maximum is 2^22. Unused bit is 23 to 32 bit.
 *   Therefor, We use 23 to 32 bit. This do not overlap with other clients.
 *   Next ID is increment of 23 to 32 bit.
 *   +------------+---------------------------+
 *   |31        23|22                        0|
 *   +------------+---------------------------+
 *   |0000 0000 00|00 0000 0000 0000 0000 0000|
 *   |<- ID     ->|<- process ID            ->|
 *   +------------+---------------------------+
 */
uint32_t QWaylandIviShellIntegration::getNextUniqueSurfaceId()
{
    const uint32_t PID_MAX_EXPONENTIATION = 22; // 22 bit shift operation
    const uint32_t ID_LIMIT = 1 << (32 - PID_MAX_EXPONENTIATION); // 10 bit is unique id
    const std::lock_guard<QRecursiveMutex> locker(m_mutex);

    if (m_lastSurfaceId == 0) {
        QByteArray env = qgetenv("QT_IVI_SURFACE_ID");
        bool ok;
        m_lastSurfaceId = env.toUInt(&ok, 10);
        if (ok)
            m_useEnvSurfaceId = true;
        else
            m_lastSurfaceId = getpid();

        return m_lastSurfaceId;
    }

    if (m_useEnvSurfaceId) {
        m_lastSurfaceId++;
    } else {
        m_surfaceNumber++;
        if (m_surfaceNumber >= ID_LIMIT) {
            qWarning("IVI surface id counter overflow\n");
            return 0;
        }
        m_lastSurfaceId += (m_surfaceNumber << PID_MAX_EXPONENTIATION);
    }

    return m_lastSurfaceId;
}

QWaylandShellSurface *QWaylandIviShellIntegration::createShellSurface(QWaylandWindow *window)
{
    if (!isActive())
        return nullptr;

    uint32_t surfaceId = getNextUniqueSurfaceId();
    if (surfaceId == 0)
        return nullptr;

    struct ivi_surface *surface = surface_create(surfaceId, window->wlSurface());
    if (!m_iviController->isActive())
        return new QWaylandIviSurface(surface, window);

    struct ::ivi_controller_surface *controller = m_iviController->ivi_controller::surface_create(surfaceId);
    QWaylandIviSurface *iviSurface = new QWaylandIviSurface(surface, window, controller);

    if (window->window()->type() == Qt::Popup) {
        QPoint transientPos = window->geometry().topLeft(); // this is absolute
        QWaylandWindow *parent = window->transientParent();
        if (parent && parent->decoration()) {
            transientPos -= parent->geometry().topLeft();
            transientPos.setX(transientPos.x() + parent->decoration()->margins().left());
            transientPos.setY(transientPos.y() + parent->decoration()->margins().top());
        }
        QSize size = window->windowGeometry().size();
        iviSurface->ivi_controller_surface::set_destination_rectangle(transientPos.x(),
                                                                      transientPos.y(),
                                                                      size.width(),
                                                                      size.height());
    }

    return iviSurface;
}

}

QT_END_NAMESPACE
