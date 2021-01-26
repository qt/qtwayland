/****************************************************************************
**
** Copyright (C) 2016 LG Electronics Inc, author: <mikko.levonmaa@lge.com>
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
****************************************************************************/

#ifndef QWAYLANDSHM_H
#define QWAYLANDSHM_H

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

#include <QVector>
#include <QImage>

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwayland-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShm : public QtWayland::wl_shm
{

public:

    QWaylandShm(QWaylandDisplay *display, int version, uint32_t id);
    ~QWaylandShm() override;

    bool formatSupported(wl_shm_format format) const;
    bool formatSupported(QImage::Format format) const;

    static wl_shm_format formatFrom(QImage::Format format);
    static QImage::Format formatFrom(wl_shm_format format);

protected:
    void shm_format(uint32_t format) override;

private:
    QVector<uint32_t> m_formats;

};

}

QT_END_NAMESPACE

#endif

