// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWLTEXTURESHARINGEXTENSION_P_H
#define QWLTEXTURESHARINGEXTENSION_P_H

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

#include "wayland-util.h"

#include <QtCore/QMap>
#include <QtCore/QHash>

#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QQuickImageProvider>

#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>
#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include <QtWaylandCompositor/private/qwayland-server-qt-texture-sharing-unstable-v1.h>

QT_BEGIN_NAMESPACE

namespace QtWayland
{
    class ServerBufferIntegration;
}

class QWaylandTextureSharingExtension;
class SharedTextureImageResponse;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandSharedTextureProvider : public QQuickAsyncImageProvider
{
public:
    QWaylandSharedTextureProvider();
    ~QWaylandSharedTextureProvider() override;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    void setExtensionReady(QWaylandTextureSharingExtension *extension);

private:
    QList<SharedTextureImageResponse*> m_pendingResponses;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextureSharingExtension
    : public QWaylandCompositorExtensionTemplate<QWaylandTextureSharingExtension>
    , public QtWaylandServer::zqt_texture_sharing_v1
{
    Q_OBJECT
    Q_PROPERTY(QString imageSearchPath WRITE setImageSearchPath)
public:
    QWaylandTextureSharingExtension();
    QWaylandTextureSharingExtension(QWaylandCompositor *compositor);
    ~QWaylandTextureSharingExtension() override;

    void initialize() override;

    void setImageSearchPath(const QString &path);

    static QWaylandTextureSharingExtension *self() { return s_self; }

public Q_SLOTS:
    void requestBuffer(const QString &key);

Q_SIGNALS:
     void bufferResult(const QString &key, QtWayland::ServerBuffer *buffer);

protected Q_SLOTS:
    void cleanupBuffers();

protected:
    void zqt_texture_sharing_v1_request_image(Resource *resource, const QString &key) override;
    void zqt_texture_sharing_v1_abandon_image(Resource *resource, const QString &key) override;
    void zqt_texture_sharing_v1_destroy_resource(Resource *resource) override;

    virtual bool customPixelData(const QString &key, QByteArray *data, QSize *size, uint *glInternalFormat)
    {
        Q_UNUSED(key);
        Q_UNUSED(data);
        Q_UNUSED(size);
        Q_UNUSED(glInternalFormat);
        return false;
    }

private:
    QtWayland::ServerBuffer *getBuffer(const QString &key);
    bool initServerBufferIntegration();
    QtWayland::ServerBuffer *getCompressedBuffer(const QString &key);
    QString getExistingFilePath(const QString &key) const;
    void dumpBufferInfo();

    struct BufferInfo
    {
        BufferInfo(QtWayland::ServerBuffer *b = nullptr) : buffer(b) {}
        QtWayland::ServerBuffer *buffer = nullptr;
        bool usedLocally = false;
    };

    QStringList m_image_dirs;
    QStringList m_image_suffixes;
    QHash<QString, BufferInfo> m_server_buffers;
    QtWayland::ServerBufferIntegration *m_server_buffer_integration = nullptr;

    static QWaylandTextureSharingExtension *s_self;
};

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandTextureSharingExtension)

QT_END_NAMESPACE

#endif // QWLTEXTURESHARINGEXTENSION_P_H
