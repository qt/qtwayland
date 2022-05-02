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

#ifndef QWAYLANDDND_H
#define QWAYLANDDND_H

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

#include <qpa/qplatformdrag.h>
#include <QtGui/private/qsimpledrag_p.h>

#include <QtGui/QDrag>
#include <QtCore/QMimeData>

#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
#if QT_CONFIG(draganddrop)
class Q_WAYLAND_CLIENT_EXPORT QWaylandDrag : public QBasicDrag
{
public:
    QWaylandDrag(QWaylandDisplay *display);
    ~QWaylandDrag() override;

    void updateTarget(const QString &mimeType);
    void setResponse(const QPlatformDragQtResponse &response);
    void finishDrag(const QPlatformDropQtResponse &response);

protected:
    void startDrag() override;
    void cancel() override;
    void move(const QPoint &globalPos, Qt::MouseButtons b, Qt::KeyboardModifiers mods) override;
    void drop(const QPoint &globalPos, Qt::MouseButtons b, Qt::KeyboardModifiers mods) override;
    void endDrag() override;


private:
    QWaylandDisplay *m_display = nullptr;
};
#endif
}

QT_END_NAMESPACE

#endif // QWAYLANDDND_H
