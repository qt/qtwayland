/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "wlextendedoutput.h"

#include "wlcompositor.h"
#include "wloutput.h"

namespace Wayland {

OutputExtensionGlobal::OutputExtensionGlobal(Compositor *compositor)
    : m_compositor(compositor)
{
    wl_display_add_global(compositor->wl_display(),
                          &wl_output_extension_interface,
                          this,
                          OutputExtensionGlobal::bind_func);
}

void OutputExtensionGlobal::bind_func(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_client_add_object(client,&wl_output_extension_interface,&output_extension_interface,id,data);
}

void OutputExtensionGlobal::get_extended_output(wl_client *client, wl_resource *output_extension_resource, uint32_t id, wl_resource *output_resource)
{
    Q_UNUSED(output_extension_resource);
    Output *output = static_cast<Output *>(output_resource->data);
    new ExtendedOutput(client,id,output);
}

const struct wl_output_extension_interface OutputExtensionGlobal::output_extension_interface = {
    OutputExtensionGlobal::get_extended_output
};

ExtendedOutput::ExtendedOutput(struct wl_client *client, uint32_t id, Output *output)
    : m_output(output)
{
    Q_ASSERT(m_output->extendedOutput() == 0);
    m_output->setExtendedOutput(this);
    m_extended_output_resource = wl_client_add_object(client,&wl_extended_output_interface,0,id,this);
}

void ExtendedOutput::sendOutputOrientation(Qt::ScreenOrientation orientation)
{
    int sendOpperation;
    switch (orientation) {
        case Qt::PortraitOrientation:
            sendOpperation = WL_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION;
            break;
    case Qt::LandscapeOrientation:
            sendOpperation = WL_EXTENDED_OUTPUT_ROTATION_LANDSCAPEORIENTATION;
            break;
    case Qt::InvertedPortraitOrientation:
            sendOpperation = WL_EXTENDED_OUTPUT_ROTATION_INVERTEDPORTRAITORIENTATION;
            break;
    case Qt::InvertedLandscapeOrientation:
            sendOpperation = WL_EXTENDED_OUTPUT_ROTATION_INVERTEDLANDSCAPEORIENTATION;
            break;
    default:
        sendOpperation = WL_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION;
    }
    wl_resource_post_event(m_extended_output_resource,WL_EXTENDED_OUTPUT_SET_SCREEN_ROTATION,sendOpperation);
}


}
