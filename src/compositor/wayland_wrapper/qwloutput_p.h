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

#ifndef WL_OUTPUT_H
#define WL_OUTPUT_H

#include <QtCompositor/qwaylandresourcecollection.h>

#include <QtCore/QRect>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Output;
class ExtendedOutput;

class OutputGlobal : public ResourceCollection
{
public:
    OutputGlobal();
    ~OutputGlobal();

    void setGeometry(const QRect &geometry);
    QRect geometry() const { return m_geometry; }

    int x() const { return m_geometry.x(); }
    int y() const { return m_geometry.y(); }
    QSize size() const { return m_geometry.size(); }

    void setRefreshRate(int rate);
    int refreshRate() const { return m_refreshRate; }

    Output *outputForClient(struct wl_client *client) const;

    static void output_bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);
private:
    QRect m_geometry;
    int m_refreshRate;
    int m_displayId;
    int m_numQueued;
    QList<Output *> m_outputs;
};


class Output
{
public:
    Output(OutputGlobal *outputGlobal, wl_client *client, uint32_t version, uint32_t id);
    ~Output();

    OutputGlobal *outputGlobal() const;

    ExtendedOutput *extendedOutput() const;
    void setExtendedOutput(ExtendedOutput *extendedOutput);

    struct wl_resource *handle() const;
private:
    struct wl_resource *m_output_resource;
    OutputGlobal *m_output_global;
    ExtendedOutput *m_extended_output;

};

}

QT_END_NAMESPACE

#endif //WL_OUTPUT_H
