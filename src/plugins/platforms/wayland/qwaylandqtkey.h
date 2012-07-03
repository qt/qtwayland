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

#ifndef QWAYLANDQTKEY_H
#define QWAYLANDQTKEY_H

#include "qwaylanddisplay.h"
#include <qpa/qwindowsysteminterface.h>

class wl_qtkey_extension;
class QWaylandQtKeyExtension
{
public:
    QWaylandQtKeyExtension(QWaylandDisplay *display, uint32_t id);

private:
    QWaylandDisplay *m_display;
    wl_qtkey_extension *m_qtkey;

    static const struct wl_qtkey_extension_listener qtkey_listener;

    static void handle_qtkey(void *data,
                             struct wl_qtkey_extension *ext,
                             uint32_t time,
                             uint32_t type,
                             uint32_t key,
                             uint32_t modifiers,
                             uint32_t nativeScanCode,
                             uint32_t nativeVirtualKey,
                             uint32_t nativeModifiers,
                             const char *text,
                             uint32_t autorep,
                             uint32_t count);
};

#endif // QWAYLANDQTKEY_H
