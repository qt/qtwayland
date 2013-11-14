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

#include "qwaylandextendedoutput.h"

#include "qwaylandscreen.h"

#include <qpa/qwindowsysteminterface.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QWaylandExtendedOutput::QWaylandExtendedOutput(QWaylandScreen *screen, ::qt_extended_output *extended_output)
    : QtWayland::qt_extended_output(extended_output)
    , m_screen(screen)
    , m_orientation(m_screen->orientation())
{
}

Qt::ScreenOrientation QWaylandExtendedOutput::currentOrientation() const
{
    return m_orientation;
}

void QWaylandExtendedOutput::setOrientationUpdateMask(Qt::ScreenOrientations orientations)
{
    int mask = 0;
    if (orientations & Qt::PortraitOrientation)
        mask |= QT_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION;
    if (orientations & Qt::LandscapeOrientation)
        mask |= QT_EXTENDED_OUTPUT_ROTATION_LANDSCAPEORIENTATION;
    if (orientations & Qt::InvertedPortraitOrientation)
        mask |= QT_EXTENDED_OUTPUT_ROTATION_INVERTEDPORTRAITORIENTATION;
    if (orientations & Qt::InvertedLandscapeOrientation)
        mask |= QT_EXTENDED_OUTPUT_ROTATION_INVERTEDLANDSCAPEORIENTATION;
    set_orientation_update_mask(mask);
}

void QWaylandExtendedOutput::extended_output_set_screen_rotation(int32_t rotation)
{
    switch (rotation) {
    case QT_EXTENDED_OUTPUT_ROTATION_PORTRAITORIENTATION:
        m_orientation = Qt::PortraitOrientation;
        break;
    case QT_EXTENDED_OUTPUT_ROTATION_LANDSCAPEORIENTATION:
        m_orientation = Qt::LandscapeOrientation;
        break;
    case QT_EXTENDED_OUTPUT_ROTATION_INVERTEDPORTRAITORIENTATION:
        m_orientation = Qt::InvertedPortraitOrientation;
        break;
    case QT_EXTENDED_OUTPUT_ROTATION_INVERTEDLANDSCAPEORIENTATION:
        m_orientation = Qt::InvertedLandscapeOrientation;
        break;
    default:
        m_orientation = Qt::PortraitOrientation;
        break;
    }
    QWindowSystemInterface::handleScreenOrientationChange(m_screen->screen(), m_orientation);
}

QT_END_NAMESPACE
