/****************************************************************************
**
** Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "mockfullscreenshellv1.h"
#include "mocksurface.h"

namespace Impl {

void FullScreenShellV1::zwp_fullscreen_shell_v1_present_surface(Resource *resource, struct ::wl_resource *surface, uint32_t method, struct ::wl_resource *output)
{
    Q_UNUSED(resource)
    Q_UNUSED(method)
    Q_UNUSED(output)

    m_surfaces.append(Surface::fromResource(surface));
}

} // namespace Impl
