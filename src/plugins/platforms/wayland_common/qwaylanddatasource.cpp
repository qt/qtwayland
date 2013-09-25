/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylanddatasource.h"
#include "qwaylanddataoffer.h"
#include "qwaylandinputdevice.h"
#include "qwaylandmimehelper.h"

#include <QtCore/QFile>

#include <QtCore/QDebug>

#include <unistd.h>

QT_BEGIN_NAMESPACE

void QWaylandDataSource::data_source_target(void *data,
               struct wl_data_source *wl_data_source,
               const char *mime_type)
{
    Q_UNUSED(data);
    Q_UNUSED(wl_data_source);
    Q_UNUSED(mime_type);
}

void QWaylandDataSource::data_source_send(void *data,
             struct wl_data_source *wl_data_source,
             const char *mime_type,
             int32_t fd)
{
    Q_UNUSED(wl_data_source);
    QWaylandDataSource *self = static_cast<QWaylandDataSource *>(data);
    QString mimeType = QString::fromLatin1(mime_type);
    QByteArray content = QWaylandMimeHelper::getByteArray(self->m_mime_data, mimeType);
    if (!content.isEmpty()) {
        QFile f;
        if (f.open(fd, QIODevice::WriteOnly))
            f.write(content);
    }
    close(fd);
}

void QWaylandDataSource::data_source_cancelled(void *data,
               struct wl_data_source *wl_data_source)
{
    Q_UNUSED(data);
    Q_UNUSED(wl_data_source);
}

const struct wl_data_source_listener QWaylandDataSource::data_source_listener = {
    data_source_target,
    data_source_send,
    data_source_cancelled
};

QWaylandDataSource::QWaylandDataSource(QWaylandDataDeviceManager *dndSelectionHandler, QMimeData *mimeData)
    : m_mime_data(mimeData)
{
    m_data_source = wl_data_device_manager_create_data_source(dndSelectionHandler->handle());
    wl_data_source_add_listener(m_data_source,&data_source_listener,this);

    if (!mimeData)
        return;

    QStringList formats = mimeData->formats();
    for (int i = 0; i < formats.size(); i++) {
        const char *offer = qPrintable(formats.at(i));
        wl_data_source_offer(m_data_source,offer);
    }
}

QWaylandDataSource::~QWaylandDataSource()
{
    wl_data_source_destroy(m_data_source);
}

QMimeData * QWaylandDataSource::mimeData() const
{
    return m_mime_data;
}

struct wl_data_source *QWaylandDataSource::handle() const
{
    return m_data_source;
}

QT_END_NAMESPACE
