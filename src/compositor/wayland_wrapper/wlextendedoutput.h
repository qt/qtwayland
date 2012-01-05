/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef WLEXTENDEDOUTPUT_H
#define WLEXTENDEDOUTPUT_H

#include "wayland-output-extension-server-protocol.h"

#include <QtCore/qnamespace.h>

namespace Wayland {

class Compositor;
class Output;

class OutputExtensionGlobal
{
public:
    OutputExtensionGlobal(Compositor *compositor);

private:
    Compositor *m_compositor;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);
    static void get_extended_output(struct wl_client *client,
                                        struct wl_resource *output_extension_resource,
                                        uint32_t id,
                                        struct wl_resource *output_resource);
    static const struct wl_output_extension_interface output_extension_interface;

};

class ExtendedOutput
{
public:
    ExtendedOutput(struct wl_client *client, uint32_t id, Output *output, Compositor *compositor);

    void sendOutputOrientation(Qt::ScreenOrientation orientation);

private:
    struct wl_resource *m_extended_output_resource;
    Output *m_output;
    Compositor *m_compositor;

};

}

#endif // WLEXTENDEDOUTPUT_H
