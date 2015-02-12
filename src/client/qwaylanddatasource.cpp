/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylanddatasource_p.h"
#include "qwaylanddataoffer_p.h"
#include "qwaylanddatadevicemanager_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandmimehelper.h"

#include <QtCore/QFile>

#include <QtCore/QDebug>

#include <unistd.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandDataSource::QWaylandDataSource(QWaylandDataDeviceManager *dataDeviceManager, QMimeData *mimeData)
    : QtWayland::wl_data_source(dataDeviceManager->create_data_source())
    , m_mime_data(mimeData)
{
    if (!mimeData)
        return;
    Q_FOREACH (const QString &format, mimeData->formats()) {
        offer(format);
    }
}

QWaylandDataSource::~QWaylandDataSource()
{
    destroy();
}

QMimeData * QWaylandDataSource::mimeData() const
{
    return m_mime_data;
}

void QWaylandDataSource::data_source_cancelled()
{
    Q_EMIT cancelled();
}

void QWaylandDataSource::data_source_send(const QString &mime_type, int32_t fd)
{
    QByteArray content = QWaylandMimeHelper::getByteArray(m_mime_data, mime_type);
    if (!content.isEmpty()) {
        write(fd, content.constData(), content.size());
    }
    close(fd);
}

void QWaylandDataSource::data_source_target(const QString &mime_type)
{
    Q_EMIT targetChanged(mime_type);
}

}

QT_END_NAMESPACE
