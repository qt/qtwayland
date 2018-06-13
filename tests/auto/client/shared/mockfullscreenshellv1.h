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

#ifndef MOCKFULLSCREENSHELLV1_H
#define MOCKFULLSCREENSHELLV1_H

#include <qwayland-server-fullscreen-shell-unstable-v1.h>

#include <QVector>

namespace Impl {

class Surface;
class FullScreenShellV1;

class FullScreenShellV1 : public QtWaylandServer::zwp_fullscreen_shell_v1
{
public:
    explicit FullScreenShellV1(::wl_display *display) : zwp_fullscreen_shell_v1(display, 1) {}

    QVector<Surface *> surfaces() const { return m_surfaces; }
    void removeSurface(Surface *surface) { m_surfaces.removeOne(surface); }

protected:
    void zwp_fullscreen_shell_v1_present_surface(Resource *resource, struct ::wl_resource *surface, uint32_t method, struct ::wl_resource *output) override;

private:
    QVector<Surface *> m_surfaces;
};

} // namespace Impl

#endif // MOCKFULLSCREENSHELLV1_H
