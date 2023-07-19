// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "waylandeglstreamintegration.h"
#include "waylandeglstreamcontroller.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtOpenGL/QOpenGLTexture>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOffscreenSurface>
#include <QtCore/QMutexLocker>

#include <QtGui/private/qeglstreamconvenience_p.h>
#include <qpa/qplatformnativeinterface.h>

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwlbuffermanager_p.h>
#include <QtWaylandCompositor/private/qwltextureorphanage_p.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <unistd.h>

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES     0x8D65
#endif

#ifndef EGL_WAYLAND_BUFFER_WL
#define EGL_WAYLAND_BUFFER_WL       0x31D5
#endif

#ifndef EGL_WAYLAND_EGLSTREAM_WL
#define EGL_WAYLAND_EGLSTREAM_WL    0x334B
#endif

#ifndef EGL_WAYLAND_PLANE_WL
#define EGL_WAYLAND_PLANE_WL        0x31D6
#endif

#ifndef EGL_WAYLAND_Y_INVERTED_WL
#define EGL_WAYLAND_Y_INVERTED_WL   0x31DB
#endif

#ifndef EGL_TEXTURE_RGB
#define EGL_TEXTURE_RGB             0x305D
#endif

#ifndef EGL_TEXTURE_RGBA
#define EGL_TEXTURE_RGBA            0x305E
#endif

#ifndef EGL_TEXTURE_EXTERNAL_WL
#define EGL_TEXTURE_EXTERNAL_WL     0x31DA
#endif

#ifndef EGL_TEXTURE_Y_U_V_WL
#define EGL_TEXTURE_Y_U_V_WL        0x31D7
#endif

#ifndef EGL_TEXTURE_Y_UV_WL
#define EGL_TEXTURE_Y_UV_WL         0x31D8
#endif

#ifndef EGL_TEXTURE_Y_XUXV_WL
#define EGL_TEXTURE_Y_XUXV_WL       0x31D9
#endif

#ifndef EGL_PLATFORM_X11_KHR
#define EGL_PLATFORM_X11_KHR        0x31D5
#endif

QT_BEGIN_NAMESPACE

/* Needed for compatibility with Mesa older than 10.0. */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYWAYLANDBUFFERWL_compat) (EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value);

#ifndef EGL_WL_bind_wayland_display
typedef EGLBoolean (EGLAPIENTRYP PFNEGLBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
#endif

static const char *
egl_error_string(EGLint code)
{
#define MYERRCODE(x) case x: return #x;
    switch (code) {
    MYERRCODE(EGL_SUCCESS)
    MYERRCODE(EGL_NOT_INITIALIZED)
    MYERRCODE(EGL_BAD_ACCESS)
    MYERRCODE(EGL_BAD_ALLOC)
    MYERRCODE(EGL_BAD_ATTRIBUTE)
    MYERRCODE(EGL_BAD_CONTEXT)
    MYERRCODE(EGL_BAD_CONFIG)
    MYERRCODE(EGL_BAD_CURRENT_SURFACE)
    MYERRCODE(EGL_BAD_DISPLAY)
    MYERRCODE(EGL_BAD_SURFACE)
    MYERRCODE(EGL_BAD_MATCH)
    MYERRCODE(EGL_BAD_PARAMETER)
    MYERRCODE(EGL_BAD_NATIVE_PIXMAP)
    MYERRCODE(EGL_BAD_NATIVE_WINDOW)
    MYERRCODE(EGL_CONTEXT_LOST)
    default:
        return "unknown";
    }
#undef MYERRCODE
}

struct BufferState
{
    BufferState() = default;

    EGLint egl_format = EGL_TEXTURE_EXTERNAL_WL;
    QOpenGLTexture *textures[3] = {nullptr, nullptr, nullptr};
    QOpenGLContext *texturesContext[3] = {nullptr, nullptr, nullptr};
    QMetaObject::Connection texturesAboutToBeDestroyedConnection[3] = {QMetaObject::Connection(), QMetaObject::Connection(), QMetaObject::Connection()};
    QMutex texturesLock;

    EGLStreamKHR egl_stream = EGL_NO_STREAM_KHR;

    bool isYInverted = false;
    QSize size;
};

class WaylandEglStreamClientBufferIntegrationPrivate
{
public:
    WaylandEglStreamClientBufferIntegrationPrivate() = default;

    bool ensureContext();
    bool initEglStream(WaylandEglStreamClientBuffer *buffer, struct ::wl_resource *bufferHandle);
    void setupBufferAndCleanup(BufferState *bs, QOpenGLTexture *texture, int plane);
    void handleEglstreamTexture(WaylandEglStreamClientBuffer *buffer);

    EGLDisplay egl_display = EGL_NO_DISPLAY;
    bool display_bound = false;
    ::wl_display *wlDisplay = nullptr;
    QOffscreenSurface *offscreenSurface = nullptr;
    QOpenGLContext *localContext = nullptr;

    WaylandEglStreamController *eglStreamController = nullptr;

    PFNEGLBINDWAYLANDDISPLAYWL egl_bind_wayland_display = nullptr;
    PFNEGLUNBINDWAYLANDDISPLAYWL egl_unbind_wayland_display = nullptr;
    PFNEGLQUERYWAYLANDBUFFERWL_compat egl_query_wayland_buffer = nullptr;

    QEGLStreamConvenience *funcs = nullptr;
    static WaylandEglStreamClientBufferIntegrationPrivate *get(WaylandEglStreamClientBufferIntegration *integration) {
        return shuttingDown ? nullptr : integration->d_ptr.data();
    }

    static bool shuttingDown;
};

bool WaylandEglStreamClientBufferIntegrationPrivate::shuttingDown = false;

bool WaylandEglStreamClientBufferIntegrationPrivate::ensureContext()
{
    bool localContextNeeded = false;
    if (!QOpenGLContext::currentContext()) {
        if (!localContext && QOpenGLContext::globalShareContext()) {
            localContext = new QOpenGLContext;
            localContext->setShareContext(QOpenGLContext::globalShareContext());
            localContext->create();
        }
        if (localContext) {
            if (!offscreenSurface) {
                offscreenSurface = new QOffscreenSurface;
                offscreenSurface->setFormat(localContext->format());
                offscreenSurface->create();
            }
            localContext->makeCurrent(offscreenSurface);
            localContextNeeded = true;
        }
    }
    return localContextNeeded;
}


void WaylandEglStreamClientBufferIntegrationPrivate::setupBufferAndCleanup(BufferState *bs, QOpenGLTexture *texture, int plane)
{
    QMutexLocker locker(&bs->texturesLock);

    bs->textures[plane] = texture;
    bs->texturesContext[plane] = QOpenGLContext::currentContext();

    Q_ASSERT(bs->texturesContext[plane] != nullptr);

    qCDebug(qLcWaylandCompositorHardwareIntegration)
            << Q_FUNC_INFO
            << "(eglstream) creating a cleanup-lambda for QOpenGLContext::aboutToBeDestroyed!"
            << ", texture: " << bs->textures[plane]
            << ", ctx: " << (void*)bs->texturesContext[plane];

    bs->texturesAboutToBeDestroyedConnection[plane] =
            QObject::connect(bs->texturesContext[plane], &QOpenGLContext::aboutToBeDestroyed,
                             bs->texturesContext[plane], [bs, plane]() {

        QMutexLocker locker(&bs->texturesLock);

        // See above lock - there is a chance that this has already been removed from textures[plane]!
        // Furthermore, we can trust that all the rest (e.g. disconnect) has also been properly executed!
        if (bs->textures[plane] == nullptr)
            return;

        delete bs->textures[plane];

        qCDebug(qLcWaylandCompositorHardwareIntegration)
                << Q_FUNC_INFO
                << "texture deleted due to QOpenGLContext::aboutToBeDestroyed!"
                << "Pointer (now dead) was:" << (void*)(bs->textures[plane])
                << "  Associated context (about to die too) is: " << (void*)(bs->texturesContext[plane]);

        bs->textures[plane] = nullptr;
        bs->texturesContext[plane] = nullptr;

        QObject::disconnect(bs->texturesAboutToBeDestroyedConnection[plane]);
        bs->texturesAboutToBeDestroyedConnection[plane] = QMetaObject::Connection();

    }, Qt::DirectConnection);
}

bool WaylandEglStreamClientBufferIntegrationPrivate::initEglStream(WaylandEglStreamClientBuffer *buffer, wl_resource *bufferHandle)
{
    BufferState &state = *buffer->d;
    state.egl_format = EGL_TEXTURE_EXTERNAL_WL;
    state.isYInverted = false;

    EGLNativeFileDescriptorKHR streamFd = EGL_NO_FILE_DESCRIPTOR_KHR;

    if (egl_query_wayland_buffer(egl_display, bufferHandle, EGL_WAYLAND_BUFFER_WL, &streamFd)) {
        state.egl_stream = funcs->create_stream_from_file_descriptor(egl_display, streamFd);
        close(streamFd);
    } else {
        EGLAttrib stream_attribs[] = {
            EGL_WAYLAND_EGLSTREAM_WL, (EGLAttrib)bufferHandle,
            EGL_NONE
        };
        state.egl_stream = funcs->create_stream_attrib_nv(egl_display, stream_attribs);
    }

    if (state.egl_stream == EGL_NO_STREAM_KHR) {
        qWarning("%s:%d: eglCreateStreamFromFileDescriptorKHR failed: 0x%x", Q_FUNC_INFO, __LINE__, eglGetError());
        return false;
    }

    bool usingLocalContext = ensureContext();

    Q_ASSERT(QOpenGLContext::currentContext());

    auto texture = new QOpenGLTexture(static_cast<QOpenGLTexture::Target>(GL_TEXTURE_EXTERNAL_OES));
    texture->create();
    setupBufferAndCleanup(buffer->d, texture, 0);

    texture->bind();

    auto newStream = funcs->stream_consumer_gltexture(egl_display, state.egl_stream);
    if (usingLocalContext)
        localContext->doneCurrent();

    if (!newStream) {
        EGLint code = eglGetError();
        qWarning() << "Could not initialize EGLStream:" << egl_error_string(code) << Qt::hex << (long)code;
        funcs->destroy_stream(egl_display, state.egl_stream);
        state.egl_stream = EGL_NO_STREAM_KHR;
        return false;
    }
    return true;
}

void WaylandEglStreamClientBufferIntegrationPrivate::handleEglstreamTexture(WaylandEglStreamClientBuffer *buffer)
{
    bool usingLocalContext = ensureContext();

    BufferState &state = *buffer->d;
    auto texture = state.textures[0];

    // EGLStream requires calling acquire on every frame.
    texture->bind();
    EGLint stream_state;
    funcs->query_stream(egl_display, state.egl_stream, EGL_STREAM_STATE_KHR, &stream_state);

    if (stream_state == EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR) {
        if (funcs->stream_consumer_acquire(egl_display, state.egl_stream) != EGL_TRUE)
            qWarning("%s:%d: eglStreamConsumerAcquireKHR failed: 0x%x", Q_FUNC_INFO, __LINE__, eglGetError());
    }

    if (usingLocalContext)
        localContext->doneCurrent();
}


WaylandEglStreamClientBufferIntegration::WaylandEglStreamClientBufferIntegration()
    : d_ptr(new WaylandEglStreamClientBufferIntegrationPrivate)
{
}

WaylandEglStreamClientBufferIntegration::~WaylandEglStreamClientBufferIntegration()
{
    Q_D(WaylandEglStreamClientBufferIntegration);
    WaylandEglStreamClientBufferIntegrationPrivate::shuttingDown = true;
    if (d->egl_unbind_wayland_display != nullptr && d->display_bound) {
        Q_ASSERT(d->wlDisplay != nullptr);
        if (!d->egl_unbind_wayland_display(d->egl_display, d->wlDisplay))
            qCWarning(qLcWaylandCompositorHardwareIntegration) << "eglUnbindWaylandDisplayWL failed";
    }
}

void WaylandEglStreamClientBufferIntegration::attachEglStreamConsumer(struct ::wl_resource *wl_surface, struct ::wl_resource *wl_buffer)
{
    Q_D(WaylandEglStreamClientBufferIntegration);
    Q_UNUSED(wl_surface);

    auto *clientBuffer = new WaylandEglStreamClientBuffer(this, wl_buffer);
    auto *bufferManager = QWaylandCompositorPrivate::get(m_compositor)->bufferManager();
    bufferManager->registerBuffer(wl_buffer, clientBuffer);

    d->initEglStream(clientBuffer, wl_buffer);
}

void WaylandEglStreamClientBufferIntegration::initializeHardware(struct wl_display *display)
{
    Q_D(WaylandEglStreamClientBufferIntegration);

    const bool ignoreBindDisplay = !qgetenv("QT_WAYLAND_IGNORE_BIND_DISPLAY").isEmpty();

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface) {
        qWarning("QtCompositor: Failed to initialize EGL display. No native platform interface available.");
        return;
    }

    d->egl_display = nativeInterface->nativeResourceForIntegration("EglDisplay");
    if (!d->egl_display) {
        qWarning("QtCompositor: Failed to initialize EGL display. Could not get EglDisplay for window.");
        return;
    }

    const char *extensionString = eglQueryString(d->egl_display, EGL_EXTENSIONS);
    if ((!extensionString || !strstr(extensionString, "EGL_WL_bind_wayland_display")) && !ignoreBindDisplay) {
        qWarning("QtCompositor: Failed to initialize EGL display. There is no EGL_WL_bind_wayland_display extension.");
        return;
    }

    d->egl_bind_wayland_display = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
    d->egl_unbind_wayland_display = reinterpret_cast<PFNEGLUNBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
    if ((!d->egl_bind_wayland_display || !d->egl_unbind_wayland_display) && !ignoreBindDisplay) {
        qWarning("QtCompositor: Failed to initialize EGL display. Could not find eglBindWaylandDisplayWL and eglUnbindWaylandDisplayWL.");
        return;
    }

    d->egl_query_wayland_buffer = reinterpret_cast<PFNEGLQUERYWAYLANDBUFFERWL_compat>(eglGetProcAddress("eglQueryWaylandBufferWL"));
    if (!d->egl_query_wayland_buffer) {
        qWarning("QtCompositor: Failed to initialize EGL display. Could not find eglQueryWaylandBufferWL.");
        return;
    }

    if (d->egl_bind_wayland_display && d->egl_unbind_wayland_display) {
        d->display_bound = d->egl_bind_wayland_display(d->egl_display, display);
        if (!d->display_bound)
            qCDebug(qLcWaylandCompositorHardwareIntegration) << "Wayland display already bound by other client buffer integration.";

        d->wlDisplay = display;
    }

    d->eglStreamController = new WaylandEglStreamController(display, this);

    d->funcs = new QEGLStreamConvenience;
    d->funcs->initialize(d->egl_display);
}

QtWayland::ClientBuffer *WaylandEglStreamClientBufferIntegration::createBufferFor(wl_resource *buffer)
{
    if (wl_shm_buffer_get(buffer))
        return nullptr;

    return new WaylandEglStreamClientBuffer(this, buffer);
}


WaylandEglStreamClientBuffer::WaylandEglStreamClientBuffer(WaylandEglStreamClientBufferIntegration *integration, wl_resource *buffer)
    : ClientBuffer(buffer)
    , m_integration(integration)
{
    auto *p = WaylandEglStreamClientBufferIntegrationPrivate::get(m_integration);
    d = new BufferState;
    if (buffer && !wl_shm_buffer_get(buffer)) {
        EGLint width, height;
        p->egl_query_wayland_buffer(p->egl_display, buffer, EGL_WIDTH, &width);
        p->egl_query_wayland_buffer(p->egl_display, buffer, EGL_HEIGHT, &height);
        d->size = QSize(width, height);
    }
}

WaylandEglStreamClientBuffer::~WaylandEglStreamClientBuffer()
{
    auto *p = WaylandEglStreamClientBufferIntegrationPrivate::get(m_integration);

    if (p) {
        if (d->egl_stream)
            p->funcs->destroy_stream(p->egl_display, d->egl_stream);

        QMutexLocker locker(&d->texturesLock);

        for (int i=0; i<3; i++) {
            if (d->textures[i] != nullptr) {

                qCDebug(qLcWaylandCompositorHardwareIntegration)
                        << Q_FUNC_INFO << " handing over texture!"
                        << (void*)d->textures[i] << "; " << (void*)d->texturesContext[i]
                        <<  " ... current context might be the same: " << QOpenGLContext::currentContext();

                QtWayland::QWaylandTextureOrphanage::instance()->admitTexture(
                        d->textures[i], d->texturesContext[i]);
                d->textures[i] = nullptr;               // in case the aboutToBeDestroyed lambda is called while we where here
                d->texturesContext[i] = nullptr;
                QObject::disconnect(d->texturesAboutToBeDestroyedConnection[i]);
                d->texturesAboutToBeDestroyedConnection[i] = QMetaObject::Connection();
            }
        }
    }

    delete d;
}


QWaylandBufferRef::BufferFormatEgl WaylandEglStreamClientBuffer::bufferFormatEgl() const
{
    return QWaylandBufferRef::BufferFormatEgl_EXTERNAL_OES;
}


QSize WaylandEglStreamClientBuffer::size() const
{
    return d->size;
}

QWaylandSurface::Origin WaylandEglStreamClientBuffer::origin() const
{
    return d->isYInverted ? QWaylandSurface::OriginTopLeft : QWaylandSurface::OriginBottomLeft;
}

QOpenGLTexture *WaylandEglStreamClientBuffer::toOpenGlTexture(int plane)
{
    // At this point we should have a valid OpenGL context, so it's safe to destroy textures
    QtWayland::QWaylandTextureOrphanage::instance()->deleteTextures();

    if (!m_buffer)
        return nullptr;

    return d->textures[plane];
}

void WaylandEglStreamClientBuffer::setCommitted(QRegion &damage)
{
    ClientBuffer::setCommitted(damage);
    auto *p = WaylandEglStreamClientBufferIntegrationPrivate::get(m_integration);
    p->handleEglstreamTexture(this);
}

QT_END_NAMESPACE
