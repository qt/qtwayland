/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "qwaylanddataoffer.h"
#include "qwaylanddatadevicemanager.h"

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QPlatformClipboard>

#include <QtCore/QDebug>


void QWaylandDataOffer::offer_sync_callback(void *data,
             struct wl_callback *wl_callback,
             uint32_t time)
{
    Q_UNUSED(time);

    QWaylandDataOffer *mime = static_cast<QWaylandDataOffer *>(data);
    mime->m_receiving_offers = false;
    wl_callback_destroy(wl_callback);
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

    if (!data_offer->m_receiving_offers) {
        struct wl_callback *callback = wl_display_sync(data_offer->m_display->wl_display());
        wl_callback_add_listener(callback,&offer_sync_callback_listener,data_offer);
        data_offer->m_receiving_offers = true;
    }

    data_offer->m_offered_mime_types.append(QString::fromLocal8Bit(type));
    qDebug() << data_offer->m_offered_mime_types;
}

const struct wl_data_offer_listener QWaylandDataOffer::data_offer_listener = {
    QWaylandDataOffer::offer
};

QWaylandDataOffer::QWaylandDataOffer(QWaylandDisplay *display, struct wl_data_offer *data_offer)
    : m_display(display)
    , m_receiving_offers(false)
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
