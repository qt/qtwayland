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

#include "qwaylanddataoffer.h"
#include "qwaylanddatadevicemanager.h"

#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformclipboard.h>

#include <QtCore/QDebug>

QT_USE_NAMESPACE

void QWaylandDataOffer::offer_sync_callback(void *data,
             struct wl_callback *callback,
             uint32_t time)
{
    Q_UNUSED(time);
    QWaylandDataOffer *mime = static_cast<QWaylandDataOffer *>(data);
    if (mime->m_receiveSyncCallback == callback) {
        mime->m_receiveSyncCallback = 0;
        wl_callback_destroy(callback);
    }
}

const struct wl_callback_listener QWaylandDataOffer::offer_sync_callback_listener = {
    QWaylandDataOffer::offer_sync_callback
};

void QWaylandDataOffer::offer(void *data,
              struct wl_data_offer *wl_data_offer,
              const char *type)
{
    Q_UNUSED(wl_data_offer);

    QWaylandDataOffer *data_offer = static_cast<QWaylandDataOffer *>(data);

    if (!data_offer->m_receiveSyncCallback) {
        data_offer->m_receiveSyncCallback = wl_display_sync(data_offer->m_display->wl_display());
        wl_callback_add_listener(data_offer->m_receiveSyncCallback, &offer_sync_callback_listener, data_offer);
    }

    data_offer->m_offered_mime_types.append(QString::fromLocal8Bit(type));
//    qDebug() << data_offer->m_offered_mime_types;
}

const struct wl_data_offer_listener QWaylandDataOffer::data_offer_listener = {
    QWaylandDataOffer::offer
};

QWaylandDataOffer::QWaylandDataOffer(QWaylandDisplay *display, struct wl_data_offer *data_offer)
    : m_display(display)
    , m_receiveSyncCallback(0)
{
    m_data_offer = data_offer;
    wl_data_offer_set_user_data(m_data_offer,this);
    wl_data_offer_add_listener(m_data_offer,&data_offer_listener,this);
}

QWaylandDataOffer::~QWaylandDataOffer()
{
    wl_data_offer_destroy(m_data_offer);
}

bool QWaylandDataOffer::hasFormat_sys(const QString &mimeType) const
{
    return m_offered_mime_types.contains(mimeType);
}

QStringList QWaylandDataOffer::formats_sys() const
{
    return m_offered_mime_types;
}

QVariant QWaylandDataOffer::retrieveData_sys(const QString &mimeType, QVariant::Type type) const
{
    Q_UNUSED(type);
    if (m_offered_mime_types.isEmpty())
        return QVariant();

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        qWarning("QWaylandMimeData: pipe() failed");
        return QVariant();
    }

    QByteArray mimeTypeBa = mimeType.toLatin1();
    wl_data_offer_receive(m_data_offer,mimeTypeBa.constData(),pipefd[1]);

    m_display->forceRoundTrip();
    close(pipefd[1]);

    QByteArray content;
    char buf[256];
    int n;
    while ((n = QT_READ(pipefd[0], &buf, sizeof buf)) > 0) {
        content.append(buf, n);
    }

    close(pipefd[0]);
    return content;
}

struct wl_data_offer *QWaylandDataOffer::handle() const
{
    return m_data_offer;
}
