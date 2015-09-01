/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandquickoutput.h"
#include "qwaylandquickcompositor.h"

QT_BEGIN_NAMESPACE

QWaylandQuickOutput::QWaylandQuickOutput(QWaylandOutputSpace *outputSpace, QQuickWindow *window)
    : QWaylandOutput(outputSpace, window)
    , m_updateScheduled(false)
    , m_automaticFrameCallbacks(false)
{
    connect(window, &QQuickWindow::beforeSynchronizing,
            this, &QWaylandQuickOutput::updateStarted,
            Qt::DirectConnection);

    connect(window, &QQuickWindow::beforeRendering,
            this, &QWaylandQuickOutput::doFrameCallbacks);
}

QQuickWindow *QWaylandQuickOutput::quickWindow() const
{
    return static_cast<QQuickWindow *>(window());
}

void QWaylandQuickOutput::update()
{
    if (!m_updateScheduled) {
        quickWindow()->update();
        m_updateScheduled = true;
    }
}

bool QWaylandQuickOutput::automaticFrameCallbacks() const
{
    return m_automaticFrameCallbacks;
}

void QWaylandQuickOutput::setAutomaticFrameCallbacks(bool automatic)
{
    m_automaticFrameCallbacks = automatic;
}

void QWaylandQuickOutput::updateStarted()
{
    m_updateScheduled = false;
    frameStarted();
    compositor()->cleanupGraphicsResources();
}

void QWaylandQuickOutput::doFrameCallbacks()
{
    if (m_automaticFrameCallbacks)
        sendFrameCallbacks();
}
QT_END_NAMESPACE
