/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WL_OUTPUT_H
#define WL_OUTPUT_H

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

#include <QtCompositor/qwaylandexport.h>

#include <QtCore/QRect>
#include <QtCore/QList>

#include <QtCompositor/private/qwayland-server-wayland.h>
#include <QtCompositor/qwaylandoutput.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWayland {

class Compositor;

struct OutputResource : public QtWaylandServer::wl_output::Resource
{
    OutputResource() {}
};

class Output : public QtWaylandServer::wl_output
{
public:
    explicit Output(Compositor *compositor, QWindow *window = 0);

    Compositor *compositor() const { return m_compositor; }

    QWaylandOutput *output() const { return m_output; }

    QString manufacturer() const { return m_manufacturer; }
    void setManufacturer(const QString &manufacturer);

    QString model() const { return m_model; }
    void setModel(const QString &model);

    QPoint position() const { return m_position; }
    void setPosition(const QPoint &position);

    QRect geometry() const;
    void setGeometry(const QRect &geometry);

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

    QWindow *window() const { return m_window; }

    OutputResource *outputForClient(struct wl_client *client) const;

    QWaylandOutput *waylandOutput() const { return m_output; }

    void output_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
    Resource *output_allocate() Q_DECL_OVERRIDE { return new OutputResource; }

private:
    friend class QT_PREPEND_NAMESPACE(QWaylandOutput);

    Compositor *m_compositor;
    QWindow *m_window;
    QWaylandOutput *m_output;
    QString m_manufacturer;
    QString m_model;
    QPoint m_position;
    QWaylandOutput::Mode m_mode;
    QRect m_availableGeometry;
    QSize m_physicalSize;
    QWaylandOutput::Subpixel m_subpixel;
    QWaylandOutput::Transform m_transform;
    int m_scaleFactor;
    QList<QWaylandSurface *> m_surfaces;

    void sendGeometryInfo();
};

}

QT_END_NAMESPACE

#endif //WL_OUTPUT_H
