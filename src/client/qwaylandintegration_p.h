/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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
******************************************************************************/

#ifndef QPLATFORMINTEGRATION_WAYLAND_H
#define QPLATFORMINTEGRATION_WAYLAND_H

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

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <qpa/qplatformintegration.h>
#include <QtCore/QScopedPointer>
#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandDisplay;
class QWaylandClientBufferIntegration;
class QWaylandServerBufferIntegration;
class QWaylandShellIntegration;
class QWaylandInputDeviceIntegration;
class QWaylandInputDevice;
class QWaylandScreen;
class QWaylandCursor;

class Q_WAYLAND_CLIENT_EXPORT QWaylandIntegration : public QPlatformIntegration
{
public:
    QWaylandIntegration();
    ~QWaylandIntegration() override;

    bool hasFailed() { return mFailed; }

    bool hasCapability(QPlatformIntegration::Capability cap) const override;
    QPlatformWindow *createPlatformWindow(QWindow *window) const override;
#if QT_CONFIG(opengl)
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
#endif
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;

    QAbstractEventDispatcher *createEventDispatcher() const override;
    void initialize() override;

    QPlatformFontDatabase *fontDatabase() const override;

    QPlatformNativeInterface *nativeInterface() const override;
#if QT_CONFIG(clipboard)
    QPlatformClipboard *clipboard() const override;
#endif
#if QT_CONFIG(draganddrop)
    QPlatformDrag *drag() const override;
#endif
    QPlatformInputContext *inputContext() const override;

    QVariant styleHint(StyleHint hint) const override;

#if QT_CONFIG(accessibility)
    QPlatformAccessibility *accessibility() const override;
#endif

    QPlatformServices *services() const override;

    QWaylandDisplay *display() const;

    QList<int> possibleKeys(const QKeyEvent *event) const override;

    QStringList themeNames() const override;

    QPlatformTheme *createPlatformTheme(const QString &name) const override;

#if QT_CONFIG(vulkan)
    QPlatformVulkanInstance *createPlatformVulkanInstance(QVulkanInstance *instance) const override;
#endif

    virtual QWaylandInputDevice *createInputDevice(QWaylandDisplay *display, int version, uint32_t id) const;
    virtual QWaylandScreen *createPlatformScreen(QWaylandDisplay *waylandDisplay, int version, uint32_t id) const;
    virtual QWaylandCursor *createPlatformCursor(QWaylandDisplay *display) const;

    virtual QWaylandClientBufferIntegration *clientBufferIntegration() const;
    virtual QWaylandServerBufferIntegration *serverBufferIntegration() const;
    virtual QWaylandShellIntegration *shellIntegration() const;

    void reconfigureInputContext();

protected:
    // NOTE: mDisplay *must* be destructed after mDrag and mClientBufferIntegration
    // and mShellIntegration.
    // Do not move this definition into the private section at the bottom.
    QScopedPointer<QWaylandDisplay> mDisplay;

protected:
    virtual QPlatformNativeInterface *createPlatformNativeInterface();

    QScopedPointer<QWaylandClientBufferIntegration> mClientBufferIntegration;
    QScopedPointer<QWaylandServerBufferIntegration> mServerBufferIntegration;
    QScopedPointer<QWaylandShellIntegration> mShellIntegration;
    QScopedPointer<QWaylandInputDeviceIntegration> mInputDeviceIntegration;

    QScopedPointer<QPlatformInputContext> mInputContext;

private:
    void initializePlatform();
    void initializeClientBufferIntegration();
    void initializeServerBufferIntegration();
    void initializeShellIntegration();
    void initializeInputDeviceIntegration();
    QWaylandShellIntegration *createShellIntegration(const QString& interfaceName);

    QScopedPointer<QPlatformFontDatabase> mFontDb;
#if QT_CONFIG(clipboard)
    QScopedPointer<QPlatformClipboard> mClipboard;
#endif
#if QT_CONFIG(draganddrop)
    QScopedPointer<QPlatformDrag> mDrag;
#endif
    QScopedPointer<QPlatformNativeInterface> mNativeInterface;
#if QT_CONFIG(accessibility)
    mutable QScopedPointer<QPlatformAccessibility> mAccessibility;
#endif
    bool mFailed = false;
    QMutex mClientBufferInitLock;
    bool mClientBufferIntegrationInitialized = false;
    bool mServerBufferIntegrationInitialized = false;
    bool mShellIntegrationInitialized = false;

    friend class QWaylandDisplay;
};

}

QT_END_NAMESPACE

#endif
