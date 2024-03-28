// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "wayland-wayland-client-protocol.h"
#include <qwayland-xdg-shell.h>
#include <wayland-ivi-application-client-protocol.h>
#include "wayland-viewporter-client-protocol.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol.h"

#include <QObject>
#include <QImage>
#include <QRect>
#include <QList>
#include <QtCore/QMap>
#include <QWaylandOutputMode>

#include "mockxdgoutputv1.h"

class MockSeat;

class ShmBuffer
{
public:
    ShmBuffer(const QSize &size, wl_shm *shm);
    ~ShmBuffer();

    struct wl_buffer *handle = nullptr;
    struct wl_shm_pool *shm_pool = nullptr;
    QImage image;
};

class MockClient : public QObject
{
    Q_OBJECT

public:
    MockClient();
    ~MockClient() override;

    wl_surface *createSurface();
    wl_shell_surface *createShellSurface(wl_surface *surface);
    xdg_surface *createXdgSurface(wl_surface *surface);
    xdg_toplevel *createXdgToplevel(xdg_surface *xdgSurface);
    ivi_surface *createIviSurface(wl_surface *surface, uint iviId);
    zwp_idle_inhibitor_v1 *createIdleInhibitor(wl_surface *surface);
    MockXdgOutputV1 *createXdgOutput(wl_output *output);

    wl_display *display = nullptr;
    wl_compositor *compositor = nullptr;
    QMap<uint, wl_output *> m_outputs;
    QMap<wl_output *, MockXdgOutputV1 *> m_xdgOutputs;
    wl_shm *shm = nullptr;
    wl_registry *registry = nullptr;
    wl_shell *wlshell = nullptr;
    xdg_wm_base *xdgWmBase = nullptr;
    wp_viewporter *viewporter = nullptr;
    ivi_application *iviApplication = nullptr;
    zwp_idle_inhibit_manager_v1 *idleInhibitManager = nullptr;
    QtWayland::zxdg_output_manager_v1 *xdgOutputManager = nullptr;

    QList<MockSeat *> m_seats;

    QRect geometry;
    QSize resolution;
    int refreshRate = -1;
    QWaylandOutputMode currentMode;
    QWaylandOutputMode preferredMode;
    QList<QWaylandOutputMode> modes;

    int fd;
    int error = 0 /* means no error according to spec */;
    struct {
        uint id = 0;
        uint code = 0;
        const wl_interface *interface = nullptr;
    } protocolError;

private slots:
    void readEvents();
    void flushDisplay();

private:
    static MockClient *resolve(void *data) { return static_cast<MockClient *>(data); }
    static const struct wl_registry_listener registryListener;
    static void handleGlobal(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
    static void handleGlobalRemove(void *data, struct wl_registry *wl_registry, uint32_t id);
    static int sourceUpdate(uint32_t mask, void *data);

    static void outputGeometryEvent(void *data,
                                    wl_output *output,
                                    int32_t x, int32_t y,
                                    int32_t width, int32_t height,
                                    int subpixel,
                                    const char *make,
                                    const char *model,
                                    int32_t transform);

    static void outputModeEvent(void *data,
                                wl_output *wl_output,
                                uint32_t flags,
                                int width,
                                int height,
                                int refreshRate);
    static void outputDone(void *data, wl_output *output);
    static void outputScale(void *data, wl_output *output, int factor);
    static void outputName(void *data, wl_output *output, const char *name);
    static void outputDesc(void *data, wl_output *output, const char *desc);

    void handleGlobal(uint32_t id, const QByteArray &interface);
    void handleGlobalRemove(uint32_t id);

    static const wl_output_listener outputListener;
};

