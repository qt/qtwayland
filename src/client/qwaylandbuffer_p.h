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

#ifndef QWAYLANDBUFFER_H
#define QWAYLANDBUFFER_H

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

#include <QtWaylandClient/qtwaylandclientglobal.h>

#include <QtCore/QSize>
#include <QtCore/QRect>

#include <QtWaylandClient/private/wayland-wayland-client-protocol.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class Q_WAYLAND_CLIENT_EXPORT QWaylandBuffer {
public:
    QWaylandBuffer();
    virtual ~QWaylandBuffer();
    void init(wl_buffer *buf);

    wl_buffer *buffer() {return mBuffer;}
    virtual QSize size() const = 0;
    virtual int scale() const { return 1; }

    void setBusy() { mBusy = true; }
    bool busy() const { return mBusy; }

    void setCommitted() { mCommitted = true; }
    bool committed() const { return mCommitted; }

protected:
    struct wl_buffer *mBuffer = nullptr;

private:
    bool mBusy = false;
    bool mCommitted = false;

    static void release(void *data, wl_buffer *);
    static const wl_buffer_listener listener;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDBUFFER_H
