// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDQTSURFACE_H
#define QWAYLANDQTSURFACE_H

#include <QtCore/qpoint.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include "qwayland-qt-shell-unstable-v1.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;

class Q_WAYLANDCLIENT_EXPORT QWaylandQtSurface : public QWaylandShellSurface
        , public QtWayland::zqt_shell_surface_v1
{
public:
    QWaylandQtSurface(struct ::zqt_shell_surface_v1 *shell_surface, QWaylandWindow *window);
    ~QWaylandQtSurface() override;

    void applyConfigure() override;
    void setWindowPosition(const QPoint &position) override;
    void setWindowSize(const QSize &size) override;

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

    std::any surfaceRole() const override { return object(); };

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
