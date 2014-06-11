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

#include "qwlextendedoutput_p.h"

#include "qwlcompositor_p.h"
#include "qwlsurface_p.h"
#include "qwloutput_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

OutputExtensionGlobal::OutputExtensionGlobal(Compositor *compositor)
    : QtWaylandServer::qt_output_extension(compositor->wl_display())
    , m_compositor(compositor)
{
}

void OutputExtensionGlobal::output_extension_get_extended_output(qt_output_extension::Resource *resource, uint32_t id, wl_resource *output_resource)
{
    Output *output = static_cast<Output *>(OutputGlobal::Resource::fromResource(output_resource));
    Q_ASSERT(output->extendedOutput == 0);

    ExtendedOutput *extendedOutput = static_cast<ExtendedOutput *>(qt_extended_output::add(resource->client(), id));

    Q_ASSERT(!output->extendedOutput);
    output->extendedOutput = extendedOutput;
    extendedOutput->output = output;

    extendedOutput->sendOutputOrientation(m_compositor->screenOrientation());
}

void OutputExtensionGlobal::extended_output_set_orientation_update_mask(qt_extended_output::Resource *resource,
                                                                        int32_t orientation_update_mask)
{
    ExtendedOutput *output = static_cast<ExtendedOutput *>(resource);

    Qt::ScreenOrientations mask = 0;
    if (orientation_update_mask & QT_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION)
        mask |= Qt::PortraitOrientation;
    if (orientation_update_mask & QT_EXTENDED_OUTPUT_ROTATION_LANDSCAPEORIENTATION)
        mask |= Qt::LandscapeOrientation;
    if (orientation_update_mask & QT_EXTENDED_OUTPUT_ROTATION_INVERTEDPORTRAITORIENTATION)
        mask |= Qt::InvertedPortraitOrientation;
    if (orientation_update_mask & QT_EXTENDED_OUTPUT_ROTATION_INVERTEDLANDSCAPEORIENTATION)
        mask |= Qt::InvertedLandscapeOrientation;

    Qt::ScreenOrientations oldMask = output->orientationUpdateMask;
    output->orientationUpdateMask = mask;

    if (mask != oldMask) {
        QList<Surface*> surfaces = m_compositor->surfacesForClient(resource->client());
        foreach (Surface *surface, surfaces) {
            if (surface->waylandSurface())
                emit surface->waylandSurface()->orientationUpdateMaskChanged();
        }
    }
}

void ExtendedOutput::sendOutputOrientation(Qt::ScreenOrientation orientation)
{
    int sendOperation;
    switch (orientation) {
        case Qt::PortraitOrientation:
            sendOperation = QT_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION;
            break;
    case Qt::LandscapeOrientation:
            sendOperation = QT_EXTENDED_OUTPUT_ROTATION_LANDSCAPEORIENTATION;
            break;
    case Qt::InvertedPortraitOrientation:
            sendOperation = QT_EXTENDED_OUTPUT_ROTATION_INVERTEDPORTRAITORIENTATION;
            break;
    case Qt::InvertedLandscapeOrientation:
            sendOperation = QT_EXTENDED_OUTPUT_ROTATION_INVERTEDLANDSCAPEORIENTATION;
            break;
    default:
        sendOperation = QT_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION;
    }

    extended_output_object->send_set_screen_rotation(handle, sendOperation);
}

}

QT_END_NAMESPACE
