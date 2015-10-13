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

#ifndef QWAYLANDDATAOFFER_H
#define QWAYLANDDATAOFFER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/private/qdnd_p.h>

#include <QtWaylandClient/private/qwaylandclientexport_p.h>
#include <QtWaylandClient/private/qwayland-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandMimeData;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDataOffer : public QtWayland::wl_data_offer
{
public:
    explicit QWaylandDataOffer(QWaylandDisplay *display, struct ::wl_data_offer *offer);
    ~QWaylandDataOffer();

    QString firstFormat() const;

    QMimeData *mimeData();

protected:
    void data_offer_offer(const QString &mime_type) Q_DECL_OVERRIDE;

private:
    QScopedPointer<QWaylandMimeData> m_mimeData;
};


class QWaylandMimeData : public QInternalMimeData {
public:
    explicit QWaylandMimeData(QWaylandDataOffer *dataOffer, QWaylandDisplay *display);
    ~QWaylandMimeData();

    void appendFormat(const QString &mimeType);

protected:
    bool hasFormat_sys(const QString &mimeType) const Q_DECL_OVERRIDE;
    QStringList formats_sys() const Q_DECL_OVERRIDE;
    QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const Q_DECL_OVERRIDE;

private:
    int readData(int fd, QByteArray &data) const;

    mutable QWaylandDataOffer *m_dataOffer;
    QWaylandDisplay *m_display;
    mutable QStringList m_types;
    mutable QHash<QString, QByteArray> m_data;
};

}

QT_END_NAMESPACE

#endif
