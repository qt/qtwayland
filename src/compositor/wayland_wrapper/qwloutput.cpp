/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

#include "qwloutput_p.h"
#include "qwlextendedoutput_p.h"
#include <QGuiApplication>
#include <QtGui/QScreen>
#include <QRect>

QT_BEGIN_NAMESPACE

namespace QtWayland {

OutputGlobal::OutputGlobal(struct ::wl_display *display)
    : QtWaylandServer::wl_output(display)
    , m_displayId(-1)
    , m_numQueued(0)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    m_geometry = QRect(QPoint(0, 0), screen->availableGeometry().size());
    m_refreshRate = qRound(screen->refreshRate() * 1000.0);
}

OutputGlobal::~OutputGlobal()
{
}

void OutputGlobal::output_bind_resource(Resource *resource)
{
    wl_output_send_geometry(resource->handle, 0, 0,
                            size().width(), size().height(), 0, "", "", 0);

    wl_output_send_mode(resource->handle, WL_OUTPUT_MODE_CURRENT|WL_OUTPUT_MODE_PREFERRED,
                        size().width(), size().height(), refreshRate());
}

void OutputGlobal::setGeometry(const QRect &geometry)
{
    m_geometry = geometry;
}

void OutputGlobal::setRefreshRate(int rate)
{
    m_refreshRate = rate;
}

Output *OutputGlobal::outputForClient(wl_client *client) const
{
    return static_cast<Output *>(resourceMap().value(client));
}

} // namespace Wayland

QT_END_NAMESPACE
