// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
    if (m_display->currentInputDevice()->dataDevice()->startDrag(drag()->mimeData(), drag()->supportedActions(), icon)) {
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

    if (drag())
        drag()->deleteLater();
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

void QWaylandDrag::setResponse(bool accepted)
{
    // This method is used for old DataDevices where the drag action is not communicated
    Qt::DropAction action = defaultAction(drag()->supportedActions(), m_display->currentInputDevice()->modifiers());
    setResponse(QPlatformDropQtResponse(accepted, action));
}

void QWaylandDrag::setResponse(const QPlatformDropQtResponse &response)
{
    setCanDrop(response.isAccepted());

    if (canDrop()) {
        updateCursor(response.acceptedAction());
    } else {
        updateCursor(Qt::IgnoreAction);
    }
}

void QWaylandDrag::setDropResponse(const QPlatformDropQtResponse &response)
{
    setExecutedDropAction(response.acceptedAction());
}

void QWaylandDrag::finishDrag()
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    eventFilter(shapedPixmapWindow(), &event);

    if (drag())
        drag()->deleteLater();
}

bool QWaylandDrag::ownsDragObject() const
{
    return true;
}

}
#endif  // draganddrop
QT_END_NAMESPACE
