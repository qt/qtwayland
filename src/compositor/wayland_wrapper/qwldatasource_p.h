// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WLDATASOURCE_H
#define WLDATASOURCE_H

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

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>
#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>
#include <QObject>
#include <QtCore/QList>

QT_REQUIRE_CONFIG(wayland_datadevice);

QT_BEGIN_NAMESPACE

namespace QtWayland {

class DataOffer;
class DataDevice;
class DataDeviceManager;

class DataSource : public QObject, public QtWaylandServer::wl_data_source
{
public:
    DataSource(struct wl_client *client, uint32_t id, uint32_t time);
    ~DataSource() override;
    uint32_t time() const;
    QList<QString> mimeTypes() const;

    void accept(const QString &mimeType);
    void send(const QString &mimeType,int fd);
    void cancel();

    void setManager(DataDeviceManager *mgr);
    void setDevice(DataDevice *device);

    static DataSource *fromResource(struct ::wl_resource *resource);

protected:
    void data_source_offer(Resource *resource, const QString &mime_type) override;
    void data_source_destroy(Resource *resource) override;
    void data_source_destroy_resource(Resource *resource) override;

private:
    uint32_t m_time;
    QList<QString> m_mimeTypes;

    DataDevice *m_device = nullptr;
    DataDeviceManager *m_manager = nullptr;
};

}

QT_END_NAMESPACE

#endif // WLDATASOURCE_H
