// Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MOCKCOMPOSITOR_FULLSCREENSHELLV1_H
#define MOCKCOMPOSITOR_FULLSCREENSHELLV1_H

#include "coreprotocol.h"
#include <qwayland-server-fullscreen-shell-unstable-v1.h>

#include <QList>

namespace MockCompositor {

class Surface;
class FullScreenShellV1;

class FullScreenShellV1 : public Global, public QtWaylandServer::zwp_fullscreen_shell_v1
{
    Q_OBJECT
public:
    explicit FullScreenShellV1(CoreCompositor *compositor);

    QList<Surface *> surfaces() const { return m_surfaces; }

protected:
    void zwp_fullscreen_shell_v1_present_surface(Resource *resource, struct ::wl_resource *surface, uint32_t method, struct ::wl_resource *output) override;

private:
    QList<Surface *> m_surfaces;
};

} // namespace MockCompositor

#endif // MOCKCOMPOSITOR_FULLSCREENSHELLV1_H
