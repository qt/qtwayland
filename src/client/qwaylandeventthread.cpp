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

#include "qwaylandeventthread_p.h"
#include <QtCore/QSocketNotifier>
#include <QCoreApplication>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandEventThread::QWaylandEventThread(QObject *parent)
    : QObject(parent)
    , m_display(0)
    , m_fileDescriptor(-1)
    , m_readNotifier(0)
    , m_displayLock(new QMutex)
{
}

QWaylandEventThread::~QWaylandEventThread()
{
    delete m_displayLock;
    wl_display_disconnect(m_display);
}

void QWaylandEventThread::displayConnect()
{
    m_displayLock->lock();
    QMetaObject::invokeMethod(this, "waylandDisplayConnect", Qt::QueuedConnection);
}

// ### be careful what you do, this function may also be called from other
// threads to clean up & exit.
void QWaylandEventThread::checkError() const
{
    int ecode = wl_display_get_error(m_display);
    if ((ecode == EPIPE || ecode == ECONNRESET)) {
        // special case this to provide a nicer error
        qWarning("The Wayland connection broke. Did the Wayland compositor die?");
    } else {
        qErrnoWarning(ecode, "The Wayland connection experienced a fatal error");
    }
}

void QWaylandEventThread::readWaylandEvents()
{
    if (wl_display_prepare_read(m_display) == 0) {
        wl_display_read_events(m_display);
    }
    emit newEventsRead();
}

void QWaylandEventThread::waylandDisplayConnect()
{
    m_display = wl_display_connect(NULL);
    if (m_display == NULL) {
        qErrnoWarning(errno, "Failed to create display");
        ::exit(1);
    }
    m_displayLock->unlock();

    m_fileDescriptor = wl_display_get_fd(m_display);

    m_readNotifier = new QSocketNotifier(m_fileDescriptor, QSocketNotifier::Read, this);
    connect(m_readNotifier, SIGNAL(activated(int)), this, SLOT(readWaylandEvents()));
}

wl_display *QWaylandEventThread::display() const
{
    QMutexLocker displayLock(m_displayLock);
    return m_display;
}

}

QT_END_NAMESPACE
