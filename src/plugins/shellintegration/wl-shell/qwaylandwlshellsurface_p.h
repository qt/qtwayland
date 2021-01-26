/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QWAYLANDWLSHELLSURFACE_H
#define QWAYLANDWLSHELLSURFACE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QSize>

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;
class QWaylandExtendedSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandWlShellSurface : public QWaylandShellSurface
    , public QtWayland::wl_shell_surface
{
    Q_OBJECT
public:
    QWaylandWlShellSurface(struct ::wl_shell_surface *shell_surface, QWaylandWindow *window);
    ~QWaylandWlShellSurface() override;

    using QtWayland::wl_shell_surface::resize;
    bool resize(QWaylandInputDevice *inputDevice, Qt::Edges edges) override;

    using QtWayland::wl_shell_surface::move;
    bool move(QWaylandInputDevice *inputDevice) override;

    void setTitle(const QString & title) override;
    void setAppId(const QString &appId) override;

    void raise() override;
    void lower() override;
    void setContentOrientationMask(Qt::ScreenOrientations orientation) override;
    void setWindowFlags(Qt::WindowFlags flags) override;
    void sendProperty(const QString &name, const QVariant &value) override;

    void applyConfigure() override;
    bool wantsDecorations() const override;

protected:
    void requestWindowStates(Qt::WindowStates states) override;

private:
    static enum resize convertToResizeEdges(Qt::Edges edges);
    void setTopLevel();
    void updateTransientParent(QWindow *parent);
    void setPopup(QWaylandWindow *parent, QWaylandInputDevice *device, uint serial);

    QWaylandWindow *m_window = nullptr;
    struct {
        Qt::WindowStates states = Qt::WindowNoState;
        QSize size;
        enum resize edges = resize_none;
    } m_applied, m_pending;
    QSize m_normalSize;
    // There's really no need to have pending and applied state on wl-shell, but we do it just to
    // keep the different shell implementations more similar.
    QWaylandExtendedSurface *m_extendedWindow = nullptr;

    void shell_surface_ping(uint32_t serial) override;
    void shell_surface_configure(uint32_t edges,
                                 int32_t width,
                                 int32_t height) override;
    void shell_surface_popup_done() override;

    friend class QWaylandWindow;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDSHELLSURFACE_H
