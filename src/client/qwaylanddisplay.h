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

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qwaylandclientexport.h>

struct wl_cursor_image;

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
class QWaylandTouchExtension;
class QWaylandQtKeyExtension;
class QWaylandWindow;
class QWaylandEventThread;
class QWaylandIntegration;

namespace QtWayland {
    class qt_output_extension;
    class qt_shell;
    class qt_sub_surface_extension;
    class qt_surface_extension;
    class wl_text_input_manager;
}

typedef void (*RegistryListener)(void *data,
                                 struct wl_registry *registry,
                                 uint32_t id,
                                 const QString &interface,
                                 uint32_t version);

class Q_WAYLAND_CLIENT_EXPORT QWaylandDisplay : public QObject, public QtWayland::wl_registry {
    Q_OBJECT

public:
    QWaylandDisplay(QWaylandIntegration *waylandIntegration);
    ~QWaylandDisplay(void);

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QWaylandScreen *screenForOutput(struct wl_output *output) const;

    struct wl_surface *createSurface(void *handle);

    QWaylandGLIntegration *glIntegration() const;

    QWaylandWindowManagerIntegration *windowManagerIntegration() const;

    void setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image);

    struct wl_display *wl_display() const { return mDisplay; }
    struct wl_event_queue *wl_event_queue() const { return mEventQueue; }
    struct ::wl_registry *wl_registry() { return object(); }

    const struct wl_compositor *wl_compositor() const { return mCompositor.object(); }
    QtWayland::wl_compositor *compositor() { return &mCompositor; }

    QtWayland::wl_shell *shell() { return mShell; }

    QList<QWaylandInputDevice *> inputDevices() const { return mInputDevices; }
    QWaylandInputDevice *defaultInputDevice() const;
    QWaylandInputDevice *currentInputDevice() const { return defaultInputDevice(); }

    QWaylandInputDevice *lastKeyboardFocusInputDevice() const;
    void setLastKeyboardFocusInputDevice(QWaylandInputDevice *device);

    QWaylandDataDeviceManager *dndSelectionHandler() const { return mDndSelectionHandler; }

    QtWayland::qt_surface_extension *windowExtension() const { return mWindowExtension; }
    QtWayland::qt_sub_surface_extension *subSurfaceExtension() const { return mSubSurfaceExtension; }
    QtWayland::qt_output_extension *outputExtension() const { return mOutputExtension; }
    QWaylandTouchExtension *touchExtension() const { return mTouchExtension; }
    QtWayland::wl_text_input_manager *textInputManager() const { return mTextInputManager; }

    /* wl_registry_add_listener does not add but rather sets a listener, so this function is used
     * to enable many listeners at once. */
    void addRegistryListener(RegistryListener listener, void *data);

    struct wl_shm *shm() const { return mShm; }

    static uint32_t currentTimeMillisec();

    void forceRoundTrip();

public slots:
    void blockingReadEvents();
    void flushRequests();

private:
    void waitForScreens();

    struct Listener {
        RegistryListener listener;
        void *data;
    };

    struct wl_display *mDisplay;
    struct wl_event_queue *mEventQueue;
    QtWayland::wl_compositor mCompositor;
    struct wl_shm *mShm;
    QThread *mEventThread;
    QWaylandEventThread *mEventThreadObject;
    QtWayland::wl_shell *mShell;
    QList<QPlatformScreen *> mScreens;
    QList<QWaylandInputDevice *> mInputDevices;
    QList<Listener> mRegistryListeners;
    QWaylandIntegration *mWaylandIntegration;
    QWaylandInputDevice *mLastKeyboardFocusInputDevice;
    QWaylandDataDeviceManager *mDndSelectionHandler;
    QtWayland::qt_surface_extension *mWindowExtension;
    QtWayland::qt_sub_surface_extension *mSubSurfaceExtension;
    QtWayland::qt_output_extension *mOutputExtension;
    QWaylandTouchExtension *mTouchExtension;
    QWaylandQtKeyExtension *mQtKeyExtension;
    QWaylandWindowManagerIntegration *mWindowManagerIntegration;
    QtWayland::wl_text_input_manager *mTextInputManager;

    QSocketNotifier *mReadNotifier;
    int mFd;
    int mWritableNotificationFd;
    bool mScreensInitialized;

    void registry_global(uint32_t id, const QString &interface, uint32_t version) Q_DECL_OVERRIDE;

    static void shellHandleConfigure(void *data, struct wl_shell *shell,
                                     uint32_t time, uint32_t edges,
                                     struct wl_surface *surface,
                                     int32_t width, int32_t height);
};

QT_END_NAMESPACE

#endif // QWAYLANDDISPLAY_H
