// Copyright (C) 2017-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDOUTPUT_P_H
#define QWAYLANDOUTPUT_P_H

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

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandClient>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandXdgOutputV1>

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QList>
#include <QtCore/QRect>

#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

struct QWaylandSurfaceViewMapper
{
    QWaylandSurfaceViewMapper()
    {}

    QWaylandSurfaceViewMapper(QWaylandSurface *s, QWaylandView *v)
        : surface(s)
        , views(1, v)
    {}

    QWaylandView *maybePrimaryView() const
    {
        for (int i = 0; i < views.size(); i++) {
            if (surface && surface->primaryView() == views.at(i))
                return views.at(i);
        }
        return nullptr;
    }

    QWaylandSurface *surface = nullptr;
    QList<QWaylandView *> views;
    bool has_entered = false;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandOutputPrivate : public QObjectPrivate, public QtWaylandServer::wl_output
{
public:
    Q_DECLARE_PUBLIC(QWaylandOutput)

    QWaylandOutputPrivate();

    ~QWaylandOutputPrivate() override;
    static QWaylandOutputPrivate *get(QWaylandOutput *output) { return output->d_func(); }

    void addView(QWaylandView *view, QWaylandSurface *surface);
    void removeView(QWaylandView *view, QWaylandSurface *surface);

    void sendGeometry(const Resource *resource);
    void sendGeometryInfo();

    void sendMode(const Resource *resource, const QWaylandOutputMode &mode);
    void sendModesInfo();

    void handleWindowPixelSizeChanged();

    QPointer<QWaylandXdgOutputV1> xdgOutput;

protected:
    void output_bind_resource(Resource *resource) override;

private:
    void _q_handleMaybeWindowPixelSizeChanged();
    void _q_handleWindowDestroyed();

    QWaylandCompositor *compositor = nullptr;
    QWindow *window = nullptr;
    QString manufacturer;
    QString model;
    QPoint position;
    QList<QWaylandOutputMode> modes;
    int currentMode = -1;
    int preferredMode = -1;
    QRect availableGeometry;
    QList<QWaylandSurfaceViewMapper> surfaceViews;
    QSize physicalSize;
    QWaylandOutput::Subpixel subpixel = QWaylandOutput::SubpixelUnknown;
    QWaylandOutput::Transform transform = QWaylandOutput::TransformNormal;
    int scaleFactor = 1;
    bool sizeFollowsWindow = false;
    bool initialized = false;
    QSize windowPixelSize;

    Q_DISABLE_COPY(QWaylandOutputPrivate)

    friend class QWaylandXdgOutputManagerV1Private;
};


QT_END_NAMESPACE

#endif  /*QWAYLANDOUTPUT_P_H*/
