/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandqtsurface_p.h"
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformwindow.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandQtSurface::QWaylandQtSurface(struct ::zqt_shell_surface_v1 *shell_surface, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , QtWayland::zqt_shell_surface_v1(shell_surface)
{
    sendSizeHints();
}

QWaylandQtSurface::~QWaylandQtSurface()
{
    zqt_shell_surface_v1::destroy();
}

void QWaylandQtSurface::resetConfiguration()
{
    m_pendingPosition = QPoint(-1, -1);
    m_pendingSize = QSize();
    m_pendingPositionValid = false;
    m_pendingStates = m_currentStates;
}

void QWaylandQtSurface::applyConfigure()
{
    if (m_pendingSize.isValid() && m_pendingPositionValid)
        setGeometryFromApplyConfigure(m_pendingPosition, m_pendingSize);
    else if (m_pendingSize.isValid())
        resizeFromApplyConfigure(m_pendingSize);
    else if (m_pendingPositionValid)
        repositionFromApplyConfigure(m_pendingPosition);

    if (m_pendingStates != m_currentStates) {
        QWindowSystemInterface::handleWindowStateChanged(platformWindow()->window(), m_pendingStates);
        m_currentStates = m_pendingStates;
    }

    ack_configure(m_currentConfigureSerial);

    resetConfiguration();
    m_currentConfigureSerial = UINT32_MAX;
}

void QWaylandQtSurface::setTitle(const QString &title)
{
    set_window_title(title);
}

void QWaylandQtSurface::zqt_shell_surface_v1_set_capabilities(uint32_t capabilities)
{
    m_capabilities = capabilities;
}

void QWaylandQtSurface::zqt_shell_surface_v1_set_position(uint32_t serial, int32_t x, int32_t y)
{
    if (serial < m_currentConfigureSerial && m_currentConfigureSerial != UINT32_MAX)
        return;

    if (serial != m_currentConfigureSerial) {
        m_currentConfigureSerial = serial;
        resetConfiguration();
    }

    m_pendingPosition = QPoint(x, y);
    m_pendingPositionValid = true;
}

void QWaylandQtSurface::zqt_shell_surface_v1_resize(uint32_t serial, int32_t width, int32_t height)
{
    if (serial < m_currentConfigureSerial && m_currentConfigureSerial != UINT32_MAX)
        return;

    if (serial != m_currentConfigureSerial) {
        m_currentConfigureSerial = serial;
        resetConfiguration();
    }

    m_pendingSize = QSize(width, height);
}

void QWaylandQtSurface::zqt_shell_surface_v1_configure(uint32_t serial)
{
    if (serial < m_currentConfigureSerial)
        return;

    if (serial > m_currentConfigureSerial) {
        m_currentConfigureSerial = serial;
        resetConfiguration();
    }

    applyConfigureWhenPossible();
}

void QWaylandQtSurface::zqt_shell_surface_v1_close()
{
    platformWindow()->window()->close();
}

void QWaylandQtSurface::zqt_shell_surface_v1_set_frame_margins(uint32_t left, uint32_t right,
                                                               uint32_t top, uint32_t bottom)
{
    QPlatformWindow *win = platformWindow();
    m_frameMargins = QMargins(left, top, right, bottom);
    m_pendingPosition = win->geometry().topLeft();
    m_pendingPositionValid = true;
    m_pendingSize = win->geometry().size();
    applyConfigureWhenPossible();
}

bool QWaylandQtSurface::requestActivate()
{
    request_activate();
    return true;
}

void QWaylandQtSurface::propagateSizeHints()
{
    sendSizeHints();
}

void QWaylandQtSurface::sendSizeHints()
{
    QPlatformWindow *win = platformWindow();
    if (win) {
        const int minWidth = qMax(0, win->windowMinimumSize().width());
        const int minHeight = qMax(0, win->windowMinimumSize().height());
        set_minimum_size(minWidth, minHeight);

        int maxWidth = qMax(0, win->windowMaximumSize().width());
        if (maxWidth == QWINDOWSIZE_MAX)
            maxWidth = -1;
        int maxHeight = qMax(0, win->windowMaximumSize().height());
        if (maxHeight == QWINDOWSIZE_MAX)
            maxHeight = -1;
        set_maximum_size(maxWidth, maxHeight);
    }
}

void QWaylandQtSurface::zqt_shell_surface_v1_set_window_state(uint32_t serial, uint32_t state)
{
    if (serial < m_currentConfigureSerial && m_currentConfigureSerial != UINT32_MAX)
        return;

    if (serial != m_currentConfigureSerial) {
        m_currentConfigureSerial = serial;
        resetConfiguration();
    }
    m_pendingStates = Qt::WindowStates(state);
}

void QWaylandQtSurface::setWindowGeometry(const QRect &rect)
{
    set_size(rect.width(), rect.height());
}

void QWaylandQtSurface::setWindowPosition(const QPoint &position)
{
    reposition(position.x(), position.y());
}

void QWaylandQtSurface::setWindowFlags(Qt::WindowFlags flags)
{
    set_window_flags(flags);
}

void QWaylandQtSurface::requestWindowStates(Qt::WindowStates states)
{
    change_window_state(states & ~Qt::WindowActive);
}

bool QWaylandQtSurface::resize(QWaylandInputDevice *inputDevice, Qt::Edges edge)
{
    if (m_capabilities & ZQT_SHELL_SURFACE_V1_CAPABILITIES_INTERACTIVE_RESIZE) {
        start_system_resize(getSerial(inputDevice), uint(edge));
        return true;
    }

    return false;
}

bool QWaylandQtSurface::move(QWaylandInputDevice *inputDevice)
{
    if (m_capabilities & ZQT_SHELL_SURFACE_V1_CAPABILITIES_INTERACTIVE_RESIZE) {
        start_system_move(getSerial(inputDevice));
        return true;
    }

    return false;
}

QMargins QWaylandQtSurface::serverSideFrameMargins() const
{
    return m_frameMargins;
}

void QWaylandQtSurface::raise()
{
    QtWayland::zqt_shell_surface_v1::raise();
}

void QWaylandQtSurface::lower()
{
    QtWayland::zqt_shell_surface_v1::lower();
}

}

QT_END_NAMESPACE
