/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
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

QT_BEGIN_NAMESPACE

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
class QWaylandQtKeyExtension;
class QWaylandWindow;
class QWaylandEventThread;

typedef void (*RegistryListener)(void *data,
                                 struct wl_registry *registry,
                                 uint32_t id,
                                 const char *interface,
                                 uint32_t version);

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

    void setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image);

    struct wl_display *wl_display() const { return mDisplay; }
    struct wl_event_queue *wl_event_queue() const { return mEventQueue; }
    struct wl_registry *wl_registry() const { return mRegistry; }
    struct wl_compositor *wl_compositor() const { return mCompositor; }

    QWaylandShell *shell() const { return mShell; }

    QList<QWaylandInputDevice *> inputDevices() const { return mInputDevices; }

    QWaylandInputDevice *lastKeyboardFocusInputDevice() const;
    void setLastKeyboardFocusInputDevice(QWaylandInputDevice *device);

    QWaylandDataDeviceManager *dndSelectionHandler() const { return mDndSelectionHandler; }

    QWaylandSurfaceExtension *windowExtension() const { return mWindowExtension; }
    QWaylandSubSurfaceExtension *subSurfaceExtension() const { return mSubSurfaceExtension; }
    QWaylandOutputExtension *outputExtension() const { return mOutputExtension; }
    QWaylandTouchExtension *touchExtension() const { return mTouchExtension; }

    /* wl_registry_add_listener does not add but rather sets a listener, so this function is used
     * to enable many listeners at once. */
    void addRegistryListener(RegistryListener listener, void *data);

    struct wl_shm *shm() const { return mShm; }

    static uint32_t currentTimeMillisec();

    void forceRoundTrip();

    void scheduleRedraw(QWaylandWindow *window);

public slots:
    void createNewScreen(struct wl_output *output);
    void blockingReadEvents();
    void flushRequests();

private:
    void waitForScreens();
    void displayHandleGlobal(uint32_t id,
                             const QByteArray &interface,
                             uint32_t version);

    struct Listener {
        RegistryListener listener;
        void *data;
    };

    struct wl_display *mDisplay;
    struct wl_event_queue *mEventQueue;
    struct wl_registry *mRegistry;
    struct wl_compositor *mCompositor;
    struct wl_shm *mShm;
    QThread *mEventThread;
    QWaylandEventThread *mEventThreadObject;
    QWaylandShell *mShell;
    QList<QPlatformScreen *> mScreens;
    QList<QWaylandInputDevice *> mInputDevices;
    QList<Listener> mRegistryListeners;
    QWaylandInputDevice *mLastKeyboardFocusInputDevice;
    QWaylandDataDeviceManager *mDndSelectionHandler;
    QWaylandSurfaceExtension *mWindowExtension;
    QWaylandSubSurfaceExtension *mSubSurfaceExtension;
    QWaylandOutputExtension *mOutputExtension;
    QWaylandTouchExtension *mTouchExtension;
    QWaylandQtKeyExtension *mQtKeyExtension;

    QSocketNotifier *mReadNotifier;
    int mFd;
    int mWritableNotificationFd;
    bool mScreensInitialized;

    static const struct wl_registry_listener registryListener;
    static const struct wl_output_listener outputListener;

    static void displayHandleGlobal(void *data,
                                    struct wl_registry *registry,
                                    uint32_t id,
                                    const char *interface,
                                    uint32_t version);
    static void outputHandleGeometry(void *data,
                                     struct wl_output *output,
                                     int32_t x, int32_t y,
                                     int32_t width, int32_t height,
                                     int subpixel,
                                     const char *make,
                                     const char *model,
                                     int32_t transform);
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
};

QT_END_NAMESPACE

#endif // QWAYLANDDISPLAY_H
