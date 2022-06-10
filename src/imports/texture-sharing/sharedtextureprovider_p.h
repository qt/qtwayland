// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef SHAREDTEXTUREPROVIDER_H
#define SHAREDTEXTUREPROVIDER_H

#include <QOpenGLFunctions>
#include <QQuickImageProvider>
#include <QtQuick/QSGTexture>
#include <QScopedPointer>
#include <QHash>

#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>

QT_BEGIN_NAMESPACE

class TextureSharingExtension;

class SharedTextureRegistry : public QObject
{
    Q_OBJECT
public:
    SharedTextureRegistry();
    ~SharedTextureRegistry() override;

    const QtWaylandClient::QWaylandServerBuffer *bufferForId(const QString &id) const;
    void requestBuffer(const QString &id);
    void abandonBuffer(const QString &id);

    static bool preinitialize();

public slots:
    void receiveBuffer(QtWaylandClient::QWaylandServerBuffer *buffer, const QString &id);

signals:
    void replyReceived(const QString &id);

private slots:
    void handleExtensionActive();

private:
    TextureSharingExtension *m_extension = nullptr;
    QHash<QString, QtWaylandClient::QWaylandServerBuffer *> m_buffers;
    QStringList m_pendingBuffers;
};

class SharedTextureProvider : public QQuickAsyncImageProvider
{
public:
    SharedTextureProvider();
    ~SharedTextureProvider() override;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    SharedTextureRegistry *m_registry = nullptr;
    bool m_sharingAvailable = false;
};

QT_END_NAMESPACE

#endif // SHAREDTEXTUREPROVIDER_H
