/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDQTSURFACE_H
#define QWAYLANDQTSURFACE_H

#include <QtCore/qpoint.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include "qwayland-qt-shell-unstable-v1.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;
class QWindow;

class Q_WAYLANDCLIENT_EXPORT QWaylandQtSurface : public QWaylandShellSurface
        , public QtWayland::zqt_shell_surface_v1
{
public:
    QWaylandQtSurface(struct ::zqt_shell_surface_v1 *shell_surface, QWaylandWindow *window);
    ~QWaylandQtSurface() override;

    void applyConfigure() override;
    void setWindowGeometry(const QRect &rect) override;
    void setWindowPosition(const QPoint &position) override;

    void setWindowFlags(Qt::WindowFlags flags) override;
    void requestWindowStates(Qt::WindowStates states) override;
    void setTitle(const QString &title) override;

    bool resize(QWaylandInputDevice *, Qt::Edges) override;
    bool move(QWaylandInputDevice *) override;
    bool requestActivate() override;

    void propagateSizeHints() override;

    QMargins serverSideFrameMargins() const override;

    void raise() override;
    void lower() override;

private:
    void resetConfiguration();
    void sendSizeHints();
    void zqt_shell_surface_v1_close() override;
    void zqt_shell_surface_v1_resize(uint32_t serial, int32_t width, int32_t height) override;
    void zqt_shell_surface_v1_set_position(uint32_t serial, int32_t x, int32_t y) override;
    void zqt_shell_surface_v1_configure(uint32_t serial) override;
    void zqt_shell_surface_v1_set_window_state(uint32_t serial, uint32_t state) override;
    void zqt_shell_surface_v1_set_frame_margins(uint32_t left, uint32_t right,
                                                uint32_t top, uint32_t bottom) override;
    void zqt_shell_surface_v1_set_capabilities(uint32_t capabilities) override;

    QSize m_pendingSize;
    QPoint m_pendingPosition = { -1, -1 };
    bool m_pendingPositionValid = false;
    Qt::WindowStates m_pendingStates = Qt::WindowNoState;
    Qt::WindowStates m_currentStates = Qt::WindowNoState;
    QMargins m_frameMargins;
    uint32_t m_currentConfigureSerial = UINT32_MAX;
    uint32_t m_capabilities = 0;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDQTSURFACE_H
