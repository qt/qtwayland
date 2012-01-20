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

#ifndef QWAYLANDDATASOURCE_H
#define QWAYLANDDATASOURCE_H

#include "qwaylanddatadevicemanager.h"

#include <wayland-client-protocol.h>

class QWaylandDataSource
{
public:
    QWaylandDataSource(QWaylandDataDeviceManager *dndSelectionHandler, QMimeData *mimeData);
    ~QWaylandDataSource();

    QMimeData *mimeData() const;

    struct wl_data_source *handle() const;
private:
    struct wl_data_source *m_data_source;
    QWaylandDisplay *m_display;
    QMimeData *m_mime_data;

    static void data_source_target(void *data,
                   struct wl_data_source *data_source,
                   const char *mime_type);
    static void data_source_send(void *data,
                 struct wl_data_source *data_source,
                 const char *mime_type,
                 int32_t fd);
    static void data_source_cancelled(void *data,
                      struct wl_data_source *data_source);
    static const struct wl_data_source_listener data_source_listener;
};

#endif // QWAYLANDDATASOURCE_H
