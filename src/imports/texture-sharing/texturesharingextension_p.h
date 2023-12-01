// Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef TEXTURESHARINGEXTENSION_H
#define TEXTURESHARINGEXTENSION_H

#include <qpa/qwindowsysteminterface.h>
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qwaylandclientextension.h>
#include "qwayland-qt-texture-sharing-unstable-v1.h"
#include "private/qglobal_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {
    class QWaylandServerBuffer;
    class QWaylandServerBufferIntegration;
};

class TextureSharingExtension : public QWaylandClientExtensionTemplate<TextureSharingExtension>
        , public QtWayland::zqt_texture_sharing_v1
{
    Q_OBJECT
public:
    TextureSharingExtension();

public Q_SLOTS:
    void requestImage(const QString &key);
    void abandonImage(const QString &key);

Q_SIGNALS:
    void bufferReceived(QtWaylandClient::QWaylandServerBuffer *buffer, const QString &key);

private:
    void zqt_texture_sharing_v1_provide_buffer(struct ::qt_server_buffer *buffer, const QString &key) override;
    void zqt_texture_sharing_v1_image_failed(const QString &key, const QString &message) override;
    QtWaylandClient::QWaylandServerBufferIntegration *m_server_buffer_integration = nullptr;
};

QT_END_NAMESPACE

#endif // TEXTURESHARINGEXTENSION_H
