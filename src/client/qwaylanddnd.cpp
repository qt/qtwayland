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

#include "qwaylanddnd_p.h"

#include "qwaylanddatadevice_p.h"
#include "qwaylanddatadevicemanager_p.h"
#include "qwaylanddataoffer_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylanddisplay_p.h"

#include <QtGui/private/qshapedpixmapdndwindow_p.h>

#include <QDebug>

QT_BEGIN_NAMESPACE
#if QT_CONFIG(draganddrop)
namespace QtWaylandClient {

QWaylandDrag::QWaylandDrag(QWaylandDisplay *display)
    : m_display(display)
{
}

QWaylandDrag::~QWaylandDrag()
{
}

void QWaylandDrag::startDrag()
{
    QBasicDrag::startDrag();
    QWaylandWindow *icon = static_cast<QWaylandWindow *>(shapedPixmapWindow()->handle());
    if (m_display->currentInputDevice()->dataDevice()->startDrag(drag()->mimeData(), icon)) {
        icon->addAttachOffset(-drag()->hotSpot());
    } else {
        // Cancelling immediately does not work, since the event loop for QDrag::exec is started
        // after this function returns.
        QMetaObject::invokeMethod(this, [this](){ cancelDrag(); }, Qt::QueuedConnection);
    }
}

void QWaylandDrag::cancel()
{
    QBasicDrag::cancel();

    m_display->currentInputDevice()->dataDevice()->cancelDrag();
}

void QWaylandDrag::move(const QPoint &globalPos, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(globalPos);
    Q_UNUSED(b);
    Q_UNUSED(mods);
    // Do nothing
}

void QWaylandDrag::drop(const QPoint &globalPos, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(globalPos);
    Q_UNUSED(b);
    Q_UNUSED(mods);
    // Do nothing
}

void QWaylandDrag::endDrag()
{
    m_display->currentInputDevice()->handleEndDrag();
}

void QWaylandDrag::updateTarget(const QString &mimeType)
{
    setCanDrop(!mimeType.isEmpty());

    if (canDrop()) {
        updateCursor(defaultAction(drag()->supportedActions(), m_display->currentInputDevice()->modifiers()));
    } else {
        updateCursor(Qt::IgnoreAction);
    }
}

void QWaylandDrag::setResponse(const QPlatformDragQtResponse &response)
{
    setCanDrop(response.isAccepted());

    if (canDrop()) {
        updateCursor(defaultAction(drag()->supportedActions(), m_display->currentInputDevice()->modifiers()));
    } else {
        updateCursor(Qt::IgnoreAction);
    }
}

void QWaylandDrag::finishDrag(const QPlatformDropQtResponse &response)
{
    setExecutedDropAction(response.acceptedAction());
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    eventFilter(shapedPixmapWindow(), &event);
}

}
#endif  // draganddrop
QT_END_NAMESPACE
