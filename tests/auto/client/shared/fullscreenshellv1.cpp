// Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "fullscreenshellv1.h"

namespace MockCompositor {

FullScreenShellV1::FullScreenShellV1(CoreCompositor *compositor)
{
    init(compositor->m_display, 1);
}

void FullScreenShellV1::zwp_fullscreen_shell_v1_present_surface(Resource *resource, struct ::wl_resource *surface, uint32_t method, struct ::wl_resource *output)
{
    Q_UNUSED(resource);
    Q_UNUSED(method);
    Q_UNUSED(output);

    m_surfaces.append(fromResource<Surface>(surface));
}

} // namespace MockCompositor
