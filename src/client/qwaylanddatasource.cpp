/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include "qwaylanddatasource_p.h"
#include "qwaylanddataoffer_p.h"
#include "qwaylanddatadevicemanager_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandmimehelper_p.h"

#include <QtCore/QFile>

#include <QtCore/QDebug>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandDataSource::QWaylandDataSource(QWaylandDataDeviceManager *dataDeviceManager, QMimeData *mimeData)
    : QtWayland::wl_data_source(dataDeviceManager->create_data_source())
    , m_mime_data(mimeData)
{
    if (!mimeData)
        return;
    const auto formats = QInternalMimeData::formatsHelper(mimeData);
    for (const QString &format : formats) {
        offer(format);
    }
}

QWaylandDataSource::~QWaylandDataSource()
{
    destroy();
}

void QWaylandDataSource::data_source_cancelled()
{
    Q_EMIT cancelled();
}

void QWaylandDataSource::data_source_send(const QString &mime_type, int32_t fd)
{
    QByteArray content = QWaylandMimeHelper::getByteArray(m_mime_data, mime_type);
    if (!content.isEmpty()) {
        // Create a sigpipe handler that does nothing, or clients may be forced to terminate
        // if the pipe is closed in the other end.
        struct sigaction action, oldAction;
        action.sa_handler = SIG_IGN;
        sigemptyset (&action.sa_mask);
        action.sa_flags = 0;

        sigaction(SIGPIPE, &action, &oldAction);
        // Some compositors (e.g., mutter) make fd with O_NONBLOCK.
        // Since wl_data_source.send describes that fd is closed here,
        // it should be done in a loop and don't have any advantage.
        // Blocking operation will be used.
        // According to fcntl(2), FSETFL ignores O_WRONLY. So this
        // call will just remove O_NONBLOCK.
        fcntl(fd, F_SETFL, O_WRONLY);
        ssize_t unused = write(fd, content.constData(), content.size());
        Q_UNUSED(unused);
        sigaction(SIGPIPE, &oldAction, nullptr);
    }
    close(fd);
}

void QWaylandDataSource::data_source_target(const QString &mime_type)
{
    Q_EMIT targetChanged(mime_type);
}

}

QT_END_NAMESPACE

#include "moc_qwaylanddatasource_p.cpp"
