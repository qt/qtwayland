/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QWAYLANDCLIPBOARD_H
#define QWAYLANDCLIPBOARD_H

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

#include <qpa/qplatformclipboard.h>
#include <QtCore/QVariant>
#include <QtCore/QMimeData>

#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_REQUIRE_CONFIG(clipboard);

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandClipboard : public QPlatformClipboard
{
public:
    QWaylandClipboard(QWaylandDisplay *display);

    ~QWaylandClipboard() override;

    QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard) override;
    void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard) override;
    bool supportsMode(QClipboard::Mode mode) const override;
    bool ownsMode(QClipboard::Mode mode) const override;

private:
    QWaylandDisplay *mDisplay = nullptr;
    QMimeData m_emptyData;
    QMimeData *m_clientClipboard[2];
};

}

QT_END_NAMESPACE

#endif // QWAYLANDCLIPBOARD_H
