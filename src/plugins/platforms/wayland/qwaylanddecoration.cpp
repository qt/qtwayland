/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylanddecoration.h"

#include "qwaylandwindow.h"
#include "qwaylandshellsurface.h"
#include "qwaylandinputdevice.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QRadialGradient>

QWaylandDecoration::QWaylandDecoration(QWaylandWindow *window)
    : m_window(window->window())
    , m_wayland_window(window)
    , m_margins(3,30,3,3)
    , m_hasSetCursor(false)
    , m_mouseButtons(Qt::NoButton)
    , m_backgroundColor(90, 90, 100)
{
    m_wayland_window->setDecoration(this);
    QTextOption option(Qt::AlignHCenter | Qt::AlignVCenter);
    option.setWrapMode(QTextOption::NoWrap);
    m_windowTitle.setTextOption(option);
}

QWaylandDecoration::~QWaylandDecoration()
{
    m_wayland_window->setDecoration(0);
}

void QWaylandDecoration::paint(QPaintDevice *device)
{
    QRect surfaceRect(QPoint(), window()->frameGeometry().size());
    QRect clips[] =
    {
        QRect(0, 0, surfaceRect.width(), margins().top()),
        QRect(0, surfaceRect.height() - margins().bottom(), surfaceRect.width(), margins().bottom()),
        QRect(0, margins().top(), margins().left(), surfaceRect.height() - margins().top() - margins().bottom()),
        QRect(surfaceRect.width() - margins().right(), margins().top(), margins().left(), surfaceRect.height() - margins().top() - margins().bottom())
    };
    QRect top = clips[0];
    QPainter p(device);
    p.setRenderHint(QPainter::Antialiasing);
    QPoint gradCenter(top.center()+ QPoint(30,60));
    QRadialGradient grad(gradCenter, top.width() / 2, gradCenter);
    QColor base(backgroundColor());
    grad.setColorAt(1, base);
    grad.setColorAt(0, base.lighter(123));
    QPainterPath roundedRect;
    roundedRect.addRoundedRect(surfaceRect, 6, 6);
    for (int i = 0; i < 4; ++i) {
        p.save();
        p.setClipRect(clips[i]);
        p.fillPath(roundedRect, grad);
        p.restore();
    }


    QString windowTitleText = window()->title();
    if (!windowTitleText.isEmpty()) {
        if (m_windowTitle.text() != windowTitleText) {
            m_windowTitle.setText(windowTitleText);
            m_windowTitle.prepare();
        }
        p.save();
        p.setClipRect(top);
        p.setPen(QColor(0xee,0xee,0xee));
        QSizeF size = m_windowTitle.size();
        int dx = (top.width() - size.width()) /2;
        int dy = (top.height()- size.height()) /2;
        QPoint windowTitlePoint(top.topLeft().x() + dx,
                 top.topLeft().y() + dy);
        p.drawStaticText(windowTitlePoint,m_windowTitle);
        p.restore();
    }
}

void QWaylandDecoration::handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::MouseButtons b, Qt::KeyboardModifiers mods)

{
    Q_UNUSED(global);
    //figure out what area mouse is in
    if (local.y() <= m_margins.top()) {
        processMouseTop(inputDevice,local,b,mods);
    } else if (local.y() > m_window->height() - m_margins.bottom()) {
        processMouseBottom(inputDevice,local,b,mods);
    } else if (local.x() <= m_margins.left()) {
        processMouseLeft(inputDevice,local,b,mods);
    } else if (local.x() > m_window->width() - m_margins.right()) {
        processMouseRight(inputDevice,local,b,mods);
    } else {
        restoreMouseCursor();
    }
    m_mouseButtons = b;
}

void QWaylandDecoration::restoreMouseCursor()
{
    if (m_hasSetCursor) {
        QGuiApplication::restoreOverrideCursor();
        m_hasSetCursor = false;
    }
}

bool QWaylandDecoration::inMouseButtonPressedState() const
{
    return m_mouseButtons & Qt::NoButton;
}

void QWaylandDecoration::startResize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize resize, Qt::MouseButtons buttons)
{
    if (isLeftClicked(buttons)) {
        m_wayland_window->shellSurface()->resize(inputDevice, resize);
        inputDevice->removeMouseButtonFromState(Qt::LeftButton);
    }
}

void QWaylandDecoration::startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons)
{
    if (isLeftClicked(buttons)) {
        m_wayland_window->shellSurface()->move(inputDevice);
        inputDevice->removeMouseButtonFromState(Qt::LeftButton);
    }
}

void QWaylandDecoration::processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods);
    if (local.y() <= m_margins.bottom()) {
        if (local.x() <= margins().left()) {
            //top left bit
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_LEFT,b);
        } else if (local.x() > m_window->width() - margins().right()) {
            //top right bit
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_RIGHT,b);
        } else {
            //top reszie bit
            overrideCursor(Qt::SplitVCursor);
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP,b);
        }
    } else {
        restoreMouseCursor();
        startMove(inputDevice,b);
    }

}

void QWaylandDecoration::processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods);
    if (local.x() <= margins().left()) {
        //bottom left bit
        startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT,b);
    } else if (local.x() > m_window->width() - margins().right()) {
        //bottom right bit
            startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT,b);
    } else {
        //bottom bit
        overrideCursor(Qt::SplitVCursor);
        startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_BOTTOM,b);
    }
}

void QWaylandDecoration::processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
    overrideCursor(Qt::SplitHCursor);
    startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_LEFT,b);
}

void QWaylandDecoration::processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
    overrideCursor(Qt::SplitHCursor);
    startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_RIGHT,b);
}

bool QWaylandDecoration::isLeftClicked(Qt::MouseButtons newMouseButtonState)
{
    if ((!m_mouseButtons & Qt::LeftButton) && (newMouseButtonState & Qt::LeftButton))
        return true;
    return false;
}

bool QWaylandDecoration::isLeftReleased(Qt::MouseButtons newMouseButtonState)
{
    if ((m_mouseButtons & Qt::LeftButton) && !(newMouseButtonState & Qt::LeftButton))
        return true;
    return false;
}

void QWaylandDecoration::setBackgroundColor(const QColor &c)
{
    m_backgroundColor = c;
}
