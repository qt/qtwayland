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

#include "qwaylanddnd_p.h"

#include "qwaylanddatadevice_p.h"
#include "qwaylanddatadevicemanager_p.h"
#include "qwaylanddataoffer_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylanddisplay_p.h"

#include <QtGui/private/qshapedpixmapdndwindow_p.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandDrag::QWaylandDrag(QWaylandDisplay *display)
    : m_display(display)
{
}

QWaylandDrag::~QWaylandDrag()
{
}

QMimeData * QWaylandDrag::platformDropData()
{
    if (drag())
        return drag()->mimeData();
    return 0;
}

void QWaylandDrag::startDrag()
{
    bool cancel = false;
    if (!shapedPixmapWindow()) {
        QBasicDrag::startDrag();
        // Don't call cancel() here, since that will hide 'shapedPixmapWindow()', and
        // QWaylandWindow::setVisible(false) will flush the window system queue,
        // ending up trying to render the window, which doesn't have a role yet,
        // and so blocking waiting for a frame callback.
        cancel = true;
    }

    QWaylandWindow *icon = static_cast<QWaylandWindow *>(shapedPixmapWindow()->handle());
    m_display->currentInputDevice()->dataDevice()->startDrag(drag()->mimeData(), icon);
    if (cancel)
        QBasicDrag::cancel();
    QBasicDrag::startDrag();
}

void QWaylandDrag::cancel()
{
    QBasicDrag::cancel();

    m_display->currentInputDevice()->dataDevice()->cancelDrag();
}

void QWaylandDrag::move(const QMouseEvent *me)
{
    Q_UNUSED(me);
    // Do nothing
}

void QWaylandDrag::drop(const QMouseEvent *me)
{
    Q_UNUSED(me);
    // Do nothing
}

void QWaylandDrag::endDrag()
{
    // Do nothing
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

QT_END_NAMESPACE
