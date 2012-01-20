/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandextendedoutput.h"

#include "qwaylandscreen.h"

#include "wayland-output-extension-client-protocol.h"

#include <QtGui/QWindowSystemInterface>

#include <QtCore/QDebug>

QWaylandOutputExtension::QWaylandOutputExtension(QWaylandDisplay *display, uint32_t id)
{
    m_output_extension = static_cast<struct wl_output_extension *>(
                wl_display_bind(display->wl_display(),id, &wl_output_extension_interface));

    QList<QPlatformScreen *> platformScreens = display->screens();
    for (int i = 0; i < platformScreens.size(); i++) {
        QWaylandScreen *waylandScreen = static_cast<QWaylandScreen *>(platformScreens.at(i));
        if (!waylandScreen->extendedOutput()) {
            QWaylandExtendedOutput *extendedOutput = getExtendedOutput(waylandScreen);
            waylandScreen->setExtendedOutput(extendedOutput);
            qDebug() << "extended output enabled";
        }
    }
}

QWaylandExtendedOutput *QWaylandOutputExtension::getExtendedOutput(QWaylandScreen *screen)
{
    qDebug() << "getExtendedOutput";
   struct wl_extended_output *extended_output =
           wl_output_extension_get_extended_output(m_output_extension,screen->output());
   return new QWaylandExtendedOutput(screen,extended_output);
}

QWaylandExtendedOutput::QWaylandExtendedOutput(QWaylandScreen *screen, wl_extended_output *extended_output)
    : m_extended_output(extended_output)
    , m_screen(screen)
    , m_orientation(m_screen->primaryOrientation())
{
    wl_extended_output_add_listener(m_extended_output,&extended_output_listener,this);
}

Qt::ScreenOrientation QWaylandExtendedOutput::currentOrientation() const
{
    return m_orientation;
}

void QWaylandExtendedOutput::set_screen_rotation(void *data, wl_extended_output *wl_extended_output, int32_t rotation)
{
    Q_UNUSED(wl_extended_output);
    QWaylandExtendedOutput *extended_output = static_cast<QWaylandExtendedOutput *>(data);
    switch (rotation) {
    case WL_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION:
        extended_output->m_orientation = Qt::PortraitOrientation;
        break;
    case WL_EXTENDED_OUTPUT_ROTATION_LANDSCAPEORIENTATION:
        extended_output->m_orientation = Qt::LandscapeOrientation;
        break;
    case WL_EXTENDED_OUTPUT_ROTATION_INVERTEDPORTRAITORIENTATION:
        extended_output->m_orientation = Qt::InvertedPortraitOrientation;
        break;
    case WL_EXTENDED_OUTPUT_ROTATION_INVERTEDLANDSCAPEORIENTATION:
        extended_output->m_orientation = Qt::InvertedLandscapeOrientation;
        break;
    default:
        extended_output->m_orientation = Qt::PortraitOrientation;
        break;
    }
    QWindowSystemInterface::handleScreenOrientationChange(extended_output->m_screen->screen(), extended_output->m_orientation);
}

const struct wl_extended_output_listener QWaylandExtendedOutput::extended_output_listener = {
    QWaylandExtendedOutput::set_screen_rotation
};
