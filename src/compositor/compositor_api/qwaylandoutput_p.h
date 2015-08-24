/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#ifndef QWAYLANDOUTPUT_P_H
#define QWAYLANDOUTPUT_P_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/QWaylandOutput>
#include <QtCompositor/QWaylandClient>
#include <QtCompositor/QWaylandOutputSpace>
#include <QtCompositor/QWaylandSurface>

#include <QtCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QRect>
#include <QtCore/QVector>

#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

struct QWaylandSurfaceViewMapper
{
    QWaylandSurfaceViewMapper()
        : surface(0)
        , views()
        , has_entered(false)
    {}

    QWaylandSurfaceViewMapper(QWaylandSurface *s, QWaylandView *v)
        : surface(s)
        , views(1, v)
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

class Q_COMPOSITOR_EXPORT QWaylandOutputPrivate : public QObjectPrivate, public QtWaylandServer::wl_output
{
public:
    QWaylandOutputPrivate(QWaylandCompositor *compositor, QWindow *window, const QString &manufacturer, const QString &model);

    ~QWaylandOutputPrivate();
    static QWaylandOutputPrivate *get(QWaylandOutput *output) { return output->d_func(); }

    void addView(QWaylandView *view, QWaylandSurface *surface);
    void removeView(QWaylandView *view, QWaylandSurface *surface);
    void sendGeometryInfo();

protected:
    void output_bind_resource(Resource *resource) Q_DECL_OVERRIDE;


private:
    QWaylandCompositor *compositor;
    QWaylandOutputSpace *outputSpace;
    QWindow *window;
    QString manufacturer;
    QString model;
    QPoint position;
    QWaylandOutput::Mode mode;
    QRect availableGeometry;
    QVector<QWaylandSurfaceViewMapper> surfaceViews;
    QSize physicalSize;
    QWaylandOutput::Subpixel subpixel;
    QWaylandOutput::Transform transform;
    int scaleFactor;
    bool sizeFollowsWindow;

    Q_DECLARE_PUBLIC(QWaylandOutput)
    Q_DISABLE_COPY(QWaylandOutputPrivate)
};


QT_END_NAMESPACE

#endif  /*QWAYLANDOUTPUT_P_H*/
