/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WL_OUTPUT_H
#define WL_OUTPUT_H

#include <QtCompositor/qwaylandexport.h>

#include <QtCompositor/QWaylandClient>
#include <QtCore/QRect>
#include <QtCore/QList>
#include <QtCore/QVector>

#include <QtCompositor/QWaylandOutputSpace>
#include <QtCompositor/QWaylandSurface>

#include <QtCompositor/private/qwayland-server-wayland.h>
#include <QtCompositor/qwaylandoutput.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWayland {

class Compositor;

struct SurfaceViewMapper
{
    SurfaceViewMapper()
        : surface(0)
        , views()
        , has_entered(false)
    {}

    QWaylandView *maybeThrottelingView() const
    {
        for (int i = 0; i < views.size(); i++) {
            if (surface && surface->throttlingView() == views.at(i))
                return views.at(i);
        }
        return Q_NULLPTR;
    }

    QWaylandSurface *surface;
    QVector<QWaylandView *> views;
    bool has_entered;
};

//Just for naming convenience
class OutputResource : public QtWaylandServer::wl_output::Resource
{
};

class Output : public QtWaylandServer::wl_output
{
public:
    Output(QWaylandOutput *output, QWaylandOutputSpace *outputSpace, QWindow *window);

    QWaylandCompositor *compositor() const { return m_outputSpace->compositor(); }

    QWaylandOutput *waylandOutput() const { return m_output; }

    QString manufacturer() const { return m_manufacturer; }
    void setManufacturer(const QString &manufacturer);

    QString model() const { return m_model; }
    void setModel(const QString &model);

    QPoint position() const { return m_position; }
    void setPosition(const QPoint &position);

    QRect geometry() const;
    void setGeometry(const QRect &geometry);
    void setWidth(int newWidth);
    void setHeight(int newHeight);
    QPoint topLeft() const { return geometry().topLeft(); }

    bool sizeFollowsWindow() const;
    void setSizeFollowsWindow(bool follow);

    QWaylandOutput::Mode mode() const { return m_mode; }
    void setMode(const QWaylandOutput::Mode &mode);

    QRect availableGeometry() const { return m_availableGeometry; }
    void setAvailableGeometry(const QRect &availableGeometry);

    QSize physicalSize() const { return m_physicalSize; }
    void setPhysicalSize(const QSize &physicalSize);

    QWaylandOutput::Subpixel subpixel() const { return m_subpixel; }
    void setSubpixel(const QWaylandOutput::Subpixel &subpixel);

    QWaylandOutput::Transform transform() const { return m_transform; }
    void setTransform(const QWaylandOutput::Transform &transform);

    int scaleFactor() const { return m_scaleFactor; }
    void setScaleFactor(int scale);

    void setOutputSpace(QWaylandOutputSpace *outputSpace, bool setOutputSpace);
    QWaylandOutputSpace *outputSpace() const { return m_outputSpace; }

    void frameStarted();
    void sendFrameCallbacks();

    void surfaceEnter(QWaylandSurface *surface);
    void surfaceLeave(QWaylandSurface *surface);

    void addView(QWaylandView *view);
    void addView(QWaylandView *view, QWaylandSurface *surface);
    void removeView(QWaylandView *view);
    void removeView(QWaylandView *view, QWaylandSurface *surface);
    void updateSurfaceForView(QWaylandView *view, QWaylandSurface *newSurface, QWaylandSurface *oldSurface);

    QWindow *window() const { return m_window; }

    OutputResource *outputForClient(QWaylandClient *client) const { return outputForClient(client->client()); }
    OutputResource *outputForClient(struct wl_client *client) const;

    void output_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
    Resource *output_allocate() Q_DECL_OVERRIDE { return new OutputResource; }

    const QVector<SurfaceViewMapper> surfaceMappers() const { return m_surfaceViews; }
private:
    friend class QT_PREPEND_NAMESPACE(QWaylandOutput);

    QWindow *m_window;
    QWaylandOutput *m_output;
    QWaylandOutputSpace *m_outputSpace;
    QString m_manufacturer;
    QString m_model;
    QPoint m_position;
    QWaylandOutput::Mode m_mode;
    QRect m_availableGeometry;
    QVector<SurfaceViewMapper> m_surfaceViews;
    QSize m_physicalSize;
    QWaylandOutput::Subpixel m_subpixel;
    QWaylandOutput::Transform m_transform;
    int m_scaleFactor;
    bool m_sizeFollowsWindow;

    void sendGeometryInfo();
};

}

QT_END_NAMESPACE

#endif //WL_OUTPUT_H
