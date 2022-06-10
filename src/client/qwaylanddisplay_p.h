// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDDISPLAY_H
#define QWAYLANDDISPLAY_H

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

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QRect>
#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>

#include <QtCore/QWaitCondition>
#include <QtCore/QLoggingCategory>

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/private/qtwaylandclientglobal_p.h>
#include <QtWaylandClient/private/qwaylandshm_p.h>

#include <qpa/qplatforminputcontextfactory_p.h>

#if QT_CONFIG(xkbcommon)
#include <QtGui/private/qxkbcommon_p.h>
#endif

struct wl_cursor_image;

QT_BEGIN_NAMESPACE

class QAbstractEventDispatcher;
class QSocketNotifier;
class QPlatformScreen;
class QPlatformPlaceholderScreen;

namespace QtWayland {
    class qt_surface_extension;
    class zwp_text_input_manager_v1;
    class zwp_text_input_manager_v2;
    class zwp_text_input_manager_v4;
    class qt_text_input_method_manager_v1;
}

namespace QtWaylandClient {

Q_WAYLANDCLIENT_EXPORT Q_DECLARE_LOGGING_CATEGORY(lcQpaWayland);

class QWaylandInputDevice;
class QWaylandBuffer;
class QWaylandScreen;
class QWaylandXdgOutputManagerV1;
class QWaylandClientBufferIntegration;
class QWaylandWindowManagerIntegration;
class QWaylandDataDeviceManager;
#if QT_CONFIG(wayland_client_primary_selection)
class QWaylandPrimarySelectionDeviceManagerV1;
#endif
#if QT_CONFIG(tabletevent)
class QWaylandTabletManagerV2;
#endif
class QWaylandPointerGestures;
class QWaylandTouchExtension;
class QWaylandQtKeyExtension;
class QWaylandWindow;
class QWaylandIntegration;
class QWaylandHardwareIntegration;
class QWaylandSurface;
class QWaylandShellIntegration;
class QWaylandCursor;
class QWaylandCursorTheme;
class EventThread;

typedef void (*RegistryListener)(void *data,
                                 struct wl_registry *registry,
                                 uint32_t id,
                                 const QString &interface,
                                 uint32_t version);

class Q_WAYLANDCLIENT_EXPORT QWaylandDisplay : public QObject, public QtWayland::wl_registry {
    Q_OBJECT

public:
    QWaylandDisplay(QWaylandIntegration *waylandIntegration);
    ~QWaylandDisplay(void) override;

    void initialize();

#if QT_CONFIG(xkbcommon)
    struct xkb_context *xkbContext() const { return mXkbContext.get(); }
#endif

    QList<QWaylandScreen *> screens() const { return mScreens; }
    QPlatformPlaceholderScreen *placeholderScreen() const { return mPlaceholderScreen; }
    void ensureScreen();

    QWaylandScreen *screenForOutput(struct wl_output *output) const;
    void handleScreenInitialized(QWaylandScreen *screen);

    struct wl_surface *createSurface(void *handle);
    struct ::wl_region *createRegion(const QRegion &qregion);
    struct ::wl_subsurface *createSubSurface(QWaylandWindow *window, QWaylandWindow *parent);

    QWaylandShellIntegration *shellIntegration() const;
    QWaylandClientBufferIntegration *clientBufferIntegration() const;
    QWaylandWindowManagerIntegration *windowManagerIntegration() const;

#if QT_CONFIG(cursor)
    QWaylandCursor *waylandCursor();
    QWaylandCursorTheme *loadCursorTheme(const QString &name, int pixelSize);
#endif
    struct wl_display *wl_display() const { return mDisplay; }
    struct ::wl_registry *wl_registry() { return object(); }

    const struct wl_compositor *wl_compositor() const { return mCompositor.object(); }
    QtWayland::wl_compositor *compositor() { return &mCompositor; }

    QList<QWaylandInputDevice *> inputDevices() const { return mInputDevices; }
    QWaylandInputDevice *defaultInputDevice() const;
    QWaylandInputDevice *currentInputDevice() const { return defaultInputDevice(); }
#if QT_CONFIG(wayland_datadevice)
    QWaylandDataDeviceManager *dndSelectionHandler() const { return mDndSelectionHandler.get(); }
#endif
#if QT_CONFIG(wayland_client_primary_selection)
    QWaylandPrimarySelectionDeviceManagerV1 *primarySelectionManager() const { return mPrimarySelectionManager.data(); }
#endif
    QtWayland::qt_surface_extension *windowExtension() const { return mWindowExtension.data(); }
#if QT_CONFIG(tabletevent)
    QWaylandTabletManagerV2 *tabletManager() const { return mTabletManager.data(); }
#endif
    QWaylandPointerGestures *pointerGestures() const { return mPointerGestures.data(); }
    QWaylandTouchExtension *touchExtension() const { return mTouchExtension.data(); }
    QtWayland::qt_text_input_method_manager_v1 *textInputMethodManager() const { return mTextInputMethodManager.data(); }
    QtWayland::zwp_text_input_manager_v1 *textInputManagerv1() const { return mTextInputManagerv1.data(); }
    QtWayland::zwp_text_input_manager_v2 *textInputManagerv2() const { return mTextInputManagerv2.data(); }
    QtWayland::zwp_text_input_manager_v4 *textInputManagerv4() const { return mTextInputManagerv4.data(); }
    QWaylandHardwareIntegration *hardwareIntegration() const { return mHardwareIntegration.data(); }
    QWaylandXdgOutputManagerV1 *xdgOutputManager() const { return mXdgOutputManager.data(); }

    struct RegistryGlobal {
        uint32_t id;
        QString interface;
        uint32_t version;
        struct ::wl_registry *registry = nullptr;
        RegistryGlobal(uint32_t id_, const QString &interface_, uint32_t version_, struct ::wl_registry *registry_)
            : id(id_), interface(interface_), version(version_), registry(registry_) { }
    };
    QList<RegistryGlobal> globals() const { return mGlobals; }
    bool hasRegistryGlobal(QStringView interfaceName) const;

    /* wl_registry_add_listener does not add but rather sets a listener, so this function is used
     * to enable many listeners at once. */
    void addRegistryListener(RegistryListener listener, void *data);
    void removeListener(RegistryListener listener, void *data);

    QWaylandShm *shm() const { return mShm.data(); }

    static uint32_t currentTimeMillisec();

    void forceRoundTrip();

    bool supportsWindowDecoration() const;

    uint32_t lastInputSerial() const { return mLastInputSerial; }
    QWaylandInputDevice *lastInputDevice() const { return mLastInputDevice; }
    QWaylandWindow *lastInputWindow() const;
    void setLastInputDevice(QWaylandInputDevice *device, uint32_t serial, QWaylandWindow *window);

    bool isWindowActivated(const QWaylandWindow *window);
    void handleWindowActivated(QWaylandWindow *window);
    void handleWindowDeactivated(QWaylandWindow *window);
    void handleKeyboardFocusChanged(QWaylandInputDevice *inputDevice);
    void handleWindowDestroyed(QWaylandWindow *window);

    wl_event_queue *frameEventQueue() { return m_frameEventQueue; };

    bool isKeyboardAvailable() const;

    void initEventThread();

public slots:
    void blockingReadEvents();
    void flushRequests();

signals:
    void globalAdded(const RegistryGlobal &global);
    void globalRemoved(const RegistryGlobal &global);

private:
    void handleWaylandSync();
    void requestWaylandSync();

    void checkTextInputProtocol();

    struct Listener {
        Listener() = default;
        Listener(RegistryListener incomingListener,
                 void* incomingData)
        : listener(incomingListener), data(incomingData)
        {}
        RegistryListener listener = nullptr;
        void *data = nullptr;
    };

    struct wl_display *mDisplay = nullptr;
    std::unique_ptr<EventThread> m_eventThread;
    wl_event_queue *m_frameEventQueue = nullptr;
    QScopedPointer<EventThread> m_frameEventQueueThread;
    QtWayland::wl_compositor mCompositor;
    QScopedPointer<QWaylandShm> mShm;
    QList<QWaylandScreen *> mWaitingScreens;
    QList<QWaylandScreen *> mScreens;
    QPlatformPlaceholderScreen *mPlaceholderScreen = nullptr;
    QList<QWaylandInputDevice *> mInputDevices;
    QList<Listener> mRegistryListeners;
    QWaylandIntegration *mWaylandIntegration = nullptr;
#if QT_CONFIG(cursor)
    struct WaylandCursorTheme {
        QString name;
        int pixelSize;
        std::unique_ptr<QWaylandCursorTheme> theme;
    };
    std::vector<WaylandCursorTheme> mCursorThemes;

    struct FindExistingCursorThemeResult {
        std::vector<WaylandCursorTheme>::const_iterator position;
        bool found;

        QWaylandCursorTheme *theme() const noexcept
        { return found ? position->theme.get() : nullptr; }
    };
    FindExistingCursorThemeResult findExistingCursorTheme(const QString &name, int pixelSize) const noexcept;

    QScopedPointer<QWaylandCursor> mCursor;
#endif
#if QT_CONFIG(wayland_datadevice)
    QScopedPointer<QWaylandDataDeviceManager> mDndSelectionHandler;
#endif
    QScopedPointer<QtWayland::qt_surface_extension> mWindowExtension;
    QScopedPointer<QtWayland::wl_subcompositor> mSubCompositor;
    QScopedPointer<QWaylandTouchExtension> mTouchExtension;
    QScopedPointer<QWaylandQtKeyExtension> mQtKeyExtension;
    QScopedPointer<QWaylandWindowManagerIntegration> mWindowManagerIntegration;
#if QT_CONFIG(tabletevent)
    QScopedPointer<QWaylandTabletManagerV2> mTabletManager;
#endif
    QScopedPointer<QWaylandPointerGestures> mPointerGestures;
#if QT_CONFIG(wayland_client_primary_selection)
    QScopedPointer<QWaylandPrimarySelectionDeviceManagerV1> mPrimarySelectionManager;
#endif
    QScopedPointer<QtWayland::qt_text_input_method_manager_v1> mTextInputMethodManager;
    QScopedPointer<QtWayland::zwp_text_input_manager_v1> mTextInputManagerv1;
    QScopedPointer<QtWayland::zwp_text_input_manager_v2> mTextInputManagerv2;
    QScopedPointer<QtWayland::zwp_text_input_manager_v4> mTextInputManagerv4;
    QScopedPointer<QWaylandHardwareIntegration> mHardwareIntegration;
    QScopedPointer<QWaylandXdgOutputManagerV1> mXdgOutputManager;
    int mFd = -1;
    int mWritableNotificationFd = -1;
    QList<RegistryGlobal> mGlobals;
    uint32_t mLastInputSerial = 0;
    QWaylandInputDevice *mLastInputDevice = nullptr;
    QPointer<QWaylandWindow> mLastInputWindow;
    QPointer<QWaylandWindow> mLastKeyboardFocus;
    QList<QWaylandWindow *> mActiveWindows;
    struct wl_callback *mSyncCallback = nullptr;
    static const wl_callback_listener syncCallbackListener;

    bool mClientSideInputContextRequested = !QPlatformInputContextFactory::requested().isNull();
    QStringList mTextInputManagerList;
    int mTextInputManagerIndex = INT_MAX;

    void registry_global(uint32_t id, const QString &interface, uint32_t version) override;
    void registry_global_remove(uint32_t id) override;

#if QT_CONFIG(xkbcommon)
    QXkbCommon::ScopedXKBContext mXkbContext;
#endif

    friend class QWaylandIntegration;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDDISPLAY_H
