/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandscreen.h"

#include "qwaylanddisplay.h"
#include "qwaylandcursor.h"
#include "qwaylandextendedoutput.h"

#include <QWindowSystemInterface>

QWaylandScreen::QWaylandScreen(QWaylandDisplay *waylandDisplay, struct wl_output *output, QRect geometry)
    : QPlatformScreen()
    , mWaylandDisplay(waylandDisplay)
    , mOutput(output)
    , mExtendedOutput(0)
    , mGeometry(geometry)
    , mDepth(32)
    , mRefreshRate(60)
    , mFormat(QImage::Format_ARGB32_Premultiplied)
    , mWaylandCursor(new QWaylandCursor(this))
{
    //maybe the global is sent after the first screen?
    if (waylandDisplay->outputExtension()) {
        mExtendedOutput = waylandDisplay->outputExtension()->getExtendedOutput(this);
    }
}

QWaylandScreen::~QWaylandScreen()
{
    delete mWaylandCursor;
}

QWaylandDisplay * QWaylandScreen::display() const
{
    return mWaylandDisplay;
}

QRect QWaylandScreen::geometry() const
{
    return mGeometry;
}

int QWaylandScreen::depth() const
{
    return mDepth;
}

QImage::Format QWaylandScreen::format() const
{
    return mFormat;
}

Qt::ScreenOrientation QWaylandScreen::orientation() const
{
    if (mExtendedOutput)
        return mExtendedOutput->currentOrientation();
    return QPlatformScreen::orientation();
}

qreal QWaylandScreen::refreshRate() const
{
    return mRefreshRate;
}

QPlatformCursor *QWaylandScreen::cursor() const
{
    return  mWaylandCursor;
}

QWaylandExtendedOutput *QWaylandScreen::extendedOutput() const
{
    return mExtendedOutput;
}

void QWaylandScreen::setExtendedOutput(QWaylandExtendedOutput *extendedOutput)
{
    Q_ASSERT(!mExtendedOutput);
    mExtendedOutput = extendedOutput;
}

QWaylandScreen * QWaylandScreen::waylandScreenFromWindow(QWindow *window)
{
    QPlatformScreen *platformScreen = QPlatformScreen::platformScreenForWindow(window);
    return static_cast<QWaylandScreen *>(platformScreen);
}

void QWaylandScreen::handleMode(const QSize &size, int refreshRate)
{
    if (size != mGeometry.size()) {
        mGeometry.setSize(size);
        QWindowSystemInterface::handleScreenGeometryChange(screen(), mGeometry);
    }

    if (refreshRate != mRefreshRate) {
        mRefreshRate = refreshRate;
        QWindowSystemInterface::handleScreenRefreshRateChange(screen(), mRefreshRate);
    }
}
