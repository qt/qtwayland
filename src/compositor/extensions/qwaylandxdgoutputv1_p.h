// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDXDGOUTPUTV1_P_H
#define QWAYLANDXDGOUTPUTV1_P_H

#include <QtCore/QHash>

#include <QWaylandOutput>
#include <QWaylandXdgOutputV1>
#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-output-unstable-v1.h>

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

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgOutputManagerV1Private
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::zxdg_output_manager_v1
{
    Q_DECLARE_PUBLIC(QWaylandXdgOutputManagerV1)
public:
    explicit QWaylandXdgOutputManagerV1Private() = default;

    void registerXdgOutput(QWaylandOutput *output, QWaylandXdgOutputV1 *xdgOutput);
    void unregisterXdgOutput(QWaylandOutput *output);

    static QWaylandXdgOutputManagerV1Private *get(QWaylandXdgOutputManagerV1 *manager) { return manager ? manager->d_func() : nullptr; }

protected:
    void zxdg_output_manager_v1_get_xdg_output(Resource *resource, uint32_t id,
                                               wl_resource *outputResource) override;

private:
    QHash<QWaylandOutput *, QWaylandXdgOutputV1 *> xdgOutputs;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgOutputV1Private
        : public QObjectPrivate
        , public QtWaylandServer::zxdg_output_v1
{
    Q_DECLARE_PUBLIC(QWaylandXdgOutputV1)
public:
    explicit QWaylandXdgOutputV1Private() = default;

    void sendLogicalPosition(const QPoint &position);
    void sendLogicalSize(const QSize &size);
    void sendDone();

    void setManager(QWaylandXdgOutputManagerV1 *manager);
    void setOutput(QWaylandOutput *output);

    static QWaylandXdgOutputV1Private *get(QWaylandXdgOutputV1 *xdgOutput) { return xdgOutput ? xdgOutput->d_func() : nullptr; }

    bool initialized = false;
    QWaylandOutput *output = nullptr;
    QWaylandXdgOutputManagerV1 *manager = nullptr;
    QPoint logicalPos;
    QSize logicalSize;
    QString name;
    QString description;
    bool needToSendDone = false;

protected:
    void zxdg_output_v1_bind_resource(Resource *resource) override;
    void zxdg_output_v1_destroy(Resource *resource) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGOUTPUTV1_P_H
