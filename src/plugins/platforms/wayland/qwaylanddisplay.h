/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDDISPLAY_H
#define QWAYLANDDISPLAY_H

#include <QtCore/QObject>
#include <QtCore/QRect>

#include <QtCore/QWaitCondition>

#include <wayland-client.h>

class QAbstractEventDispatcher;
class QWaylandInputDevice;
class QSocketNotifier;
class QWaylandBuffer;
class QPlatformScreen;
class QWaylandScreen;
class QWaylandGLIntegration;
class QWaylandWindowManagerIntegration;
class QWaylandDataDeviceManager;
class QWaylandShell;
class QWaylandSurfaceExtension;
class QWaylandSubSurfaceExtension;
class QWaylandOutputExtension;
class QWaylandTouchExtension;

class QWaylandDisplay : public QObject {
    Q_OBJECT

public:
    QWaylandDisplay(void);
    ~QWaylandDisplay(void);

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QWaylandScreen *screenForOutput(struct wl_output *output) const;

    struct wl_surface *createSurface(void *handle);

#ifdef QT_WAYLAND_GL_SUPPORT
    QWaylandGLIntegration *eglIntegration();
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    QWaylandWindowManagerIntegration *windowManagerIntegration();
#endif

    void setCursor(QWaylandBuffer *buffer, int32_t x, int32_t y);

    struct wl_display *wl_display() const { return mDisplay; }

    QWaylandShell *shell() const { return mShell; }

    QList<QWaylandInputDevice *> inputDevices() const { return mInputDevices; }

    QWaylandInputDevice *lastKeyboardFocusInputDevice() const;
    void setLastKeyboardFocusInputDevice(QWaylandInputDevice *device);

    QWaylandDataDeviceManager *dndSelectionHandler() const { return mDndSelectionHandler; }

    QWaylandSurfaceExtension *windowExtension() const { return mWindowExtension; }
    QWaylandSubSurfaceExtension *subSurfaceExtension() const { return mSubSurfaceExtension; }
    QWaylandOutputExtension *outputExtension() const { return mOutputExtension; }
    QWaylandTouchExtension *touchExtension() const { return mTouchExtension; }

    struct wl_shm *shm() const { return mShm; }

    static uint32_t currentTimeMillisec();

    void forceRoundTrip();
public slots:
    void createNewScreen(struct wl_output *output, QRect geometry);
    void readEvents();
    void blockingReadEvents();
    void flushRequests();

private:
    void waitForScreens();
    void displayHandleGlobal(uint32_t id,
                             const QByteArray &interface,
                             uint32_t version);

    struct wl_display *mDisplay;
    struct wl_compositor *mCompositor;
    struct wl_shm *mShm;
    QWaylandShell *mShell;
    QList<QPlatformScreen *> mScreens;
    QList<QWaylandInputDevice *> mInputDevices;
    QWaylandInputDevice *mLastKeyboardFocusInputDevice;
    QWaylandDataDeviceManager *mDndSelectionHandler;
    QWaylandSurfaceExtension *mWindowExtension;
    QWaylandSubSurfaceExtension *mSubSurfaceExtension;
    QWaylandOutputExtension *mOutputExtension;
    QWaylandTouchExtension *mTouchExtension;

    QSocketNotifier *mReadNotifier;
    int mFd;
    int mWritableNotificationFd;
    bool mScreensInitialized;

    uint32_t mSocketMask;


    static const struct wl_output_listener outputListener;
    static int sourceUpdate(uint32_t mask, void *data);
    static void displayHandleGlobal(struct wl_display *display,
                                    uint32_t id,
                                    const char *interface,
                                    uint32_t version, void *data);
    static void outputHandleGeometry(void *data,
                                     struct wl_output *output,
                                     int32_t x, int32_t y,
                                     int32_t width, int32_t height,
                                     int subpixel,
                                     const char *make,
                                     const char *model);
    static void mode(void *data,
                     struct wl_output *wl_output,
                     uint32_t flags,
                     int width,
                     int height,
                     int refresh);

#ifdef QT_WAYLAND_GL_SUPPORT
    QWaylandGLIntegration *mEglIntegration;
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    QWaylandWindowManagerIntegration *mWindowManagerIntegration;
#endif

    static void shellHandleConfigure(void *data, struct wl_shell *shell,
                                     uint32_t time, uint32_t edges,
                                     struct wl_surface *surface,
                                     int32_t width, int32_t height);

    static void force_roundtrip_sync_callback(void *data, struct wl_callback *wl_callback, uint32_t time);
    static const struct wl_callback_listener force_roundtrip_sync_callback_listener;
};

#endif // QWAYLANDDISPLAY_H
