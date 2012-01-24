/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDEXTENDEDOUTPUT_H
#define QWAYLANDEXTENDEDOUTPUT_H

#include "qwaylanddisplay.h"

class QWaylandExtendedOutput;

class QWaylandOutputExtension
{
public:
    QWaylandOutputExtension(QWaylandDisplay *display, uint32_t id);

    QWaylandExtendedOutput* getExtendedOutput(QWaylandScreen *screen);
private:
    struct wl_output_extension *m_output_extension;
};

class QWaylandExtendedOutput
{
public:
    QWaylandExtendedOutput(QWaylandScreen *screen, struct wl_extended_output *extended_output);

    Qt::ScreenOrientation currentOrientation() const;
private:
    struct wl_extended_output *m_extended_output;
    QWaylandScreen *m_screen;
    Qt::ScreenOrientation m_orientation;

    static void set_screen_rotation(void *data,
                             struct wl_extended_output *wl_extended_output,
                             int32_t rotation);
    static const struct wl_extended_output_listener extended_output_listener;
};


#endif // QWAYLANDEXTENDEDOUTPUT_H
