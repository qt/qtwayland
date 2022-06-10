// Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mockcompositor.h"

namespace MockCompositor {

DefaultCompositor::DefaultCompositor(CompositorType t)
    : CoreCompositor(t)
{
    {
        Lock l(this);

        // Globals: Should ideally always be at least the latest versions we support.
        // Legacy versions can override in separate tests by removing and adding.
        add<WlCompositor>();
        add<SubCompositor>();
        auto *output = add<Output>();
        output->m_data.physicalSize = output->m_data.mode.physicalSizeForDpi(96);
        add<Seat>(Seat::capability_pointer | Seat::capability_keyboard | Seat::capability_touch);
        add<WlShell>();
        add<XdgWmBase>();
        switch (m_type) {
        case CompositorType::Default:
            add<Shm>();
            break;
        case CompositorType::Legacy:
            wl_display_init_shm(m_display);
            break;
        }
        add<FullScreenShellV1>();
        add<IviApplication>();

        // TODO: other shells, viewporter, xdgoutput etc

        QObject::connect(get<WlCompositor>(), &WlCompositor::surfaceCreated, [&] (Surface *surface){
            QObject::connect(surface, &Surface::bufferCommitted, [=] {
                if (m_config.autoRelease) {
                    // Pretend we made a copy of the buffer and just release it immediately
                    surface->m_committed.buffer->send_release();
                }
                if (m_config.autoEnter && get<Output>() && surface->m_outputs.empty())
                    surface->sendEnter(get<Output>());
                wl_display_flush_clients(m_display);
            });
        });

        QObject::connect(get<XdgWmBase>(), &XdgWmBase::toplevelCreated, get<XdgWmBase>(), [&] (XdgToplevel *toplevel) {
            if (m_config.autoConfigure)
                toplevel->sendCompleteConfigure();
        }, Qt::DirectConnection);
    }
    Q_ASSERT(isClean());
}

Surface *DefaultCompositor::surface(int i)
{
    QList<Surface *> surfaces;
    switch (m_type) {
    case CompositorType::Default:
        return get<WlCompositor>()->m_surfaces.value(i, nullptr);
    case CompositorType::Legacy: {
            QList<Surface *> msurfaces = get<WlCompositor>()->m_surfaces;
            for (Surface *surface : msurfaces) {
                if (surface->isMapped()) {
                    surfaces << surface;
                }
            }
        }
        break;
    }

    if (i >= 0 && i < surfaces.size())
        return surfaces[i];

    return nullptr;
}

uint DefaultCompositor::sendXdgShellPing()
{
    warnIfNotLockedByThread(Q_FUNC_INFO);
    uint serial = nextSerial();
    auto *base = get<XdgWmBase>();
    const auto resourceMap = base->resourceMap();
    Q_ASSERT(resourceMap.size() == 1); // binding more than once shouldn't be needed
    base->send_ping(resourceMap.first()->handle, serial);
    return serial;
}

void DefaultCompositor::xdgPingAndWaitForPong()
{
    QSignalSpy pongSpy(exec([=] { return get<XdgWmBase>(); }), &XdgWmBase::pong);
    uint serial = exec([=] { return sendXdgShellPing(); });
    QTRY_COMPARE(pongSpy.count(), 1);
    QTRY_COMPARE(pongSpy.first().at(0).toUInt(), serial);
}

void DefaultCompositor::sendShellSurfaceConfigure(Surface *surface)
{
    switch (m_type) {
    case CompositorType::Default:
        break;
    case CompositorType::Legacy: {
            if (auto wlShellSurface = surface->wlShellSurface()) {
                wlShellSurface->sendConfigure(0, 0, 0);
                return;
            }
            break;
        }
    }

    qWarning() << "The mocking framework doesn't know how to send a configure event for this surface";
}

WlShellCompositor::WlShellCompositor(CompositorType t)
    : DefaultCompositor(t)
{
}

Surface *DefaultCompositor::wlSurface(int i)
{
    QList<Surface *> surfaces, msurfaces;
    msurfaces = get<WlCompositor>()->m_surfaces;
    for (Surface *surface : msurfaces) {
        if (surface->isMapped())
            surfaces << surface;
    }

    if (i >=0 && i < surfaces.size())
        return surfaces[i];

    return nullptr;
}

} // namespace MockCompositor
