/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the config.tests of the Qt Toolkit.
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

#ifndef QWAYLANDWINDOW_H
#define QWAYLANDWINDOW_H

#include <qpa/qplatformwindow.h>
#include <QtCore/QWaitCondition>

#include "qwaylanddisplay.h"

class QWaylandDisplay;
class QWaylandBuffer;
class QWaylandShellSurface;
class QWaylandExtendedSurface;
class QWaylandSubSurface;
class QWaylandDecoration;

struct wl_egl_window;

class QWaylandWindow : public QPlatformWindow
{
public:
    enum WindowType {
        Shm,
        Egl
    };

    QWaylandWindow(QWindow *window);
    ~QWaylandWindow();

    virtual WindowType windowType() const = 0;
    WId winId() const;
    void setVisible(bool visible);
    void setParent(const QPlatformWindow *parent);

    void setWindowTitle(const QString &title);

    void setGeometry(const QRect &rect);

    void configure(uint32_t edges, int32_t width, int32_t height);

    void attach(QWaylandBuffer *buffer);
    QWaylandBuffer *attached() const;

    void damage(const QRect &rect);

    void waitForFrameSync();

    QMargins frameMargins() const;

    struct wl_surface *wl_surface() const { return mSurface; }

    QWaylandShellSurface *shellSurface() const;
    QWaylandExtendedSurface *extendedWindow() const;
    QWaylandSubSurface *subSurfaceWindow() const;

    void handleContentOrientationChange(Qt::ScreenOrientation orientation);
    Qt::ScreenOrientation requestWindowOrientation(Qt::ScreenOrientation orientation);

    Qt::WindowState setWindowState(Qt::WindowState state);
    void setWindowFlags(Qt::WindowFlags flags);

    bool isExposed() const;

    QWaylandDecoration *decoration() const;
    void setDecoration(QWaylandDecoration *decoration);


    void handleMouse(QWaylandInputDevice *inputDevice,
                     ulong timestamp,
                     const QPointF & local,
                     const QPointF & global,
                     Qt::MouseButtons b,
                     Qt::KeyboardModifiers mods);
    void handleMouseEnter();
    void handleMouseLeave();
protected:
    QWaylandDisplay *mDisplay;
    struct wl_surface *mSurface;
    QWaylandShellSurface *mShellSurface;
    QWaylandExtendedSurface *mExtendedWindow;
    QWaylandSubSurface *mSubSurfaceWindow;

    QWaylandDecoration *mWindowDecoration;
    bool mMouseEventsInContentArea;
    Qt::MouseButtons mMousePressedInContentArea;

    QWaylandBuffer *mBuffer;
    WId mWindowId;
    bool mWaitingForFrameSync;
    struct wl_callback *mFrameCallback;
    QWaitCondition mFrameSyncWait;

    bool mSentInitialResize;

private:
    void handleMouseEventWithDecoration(QWaylandInputDevice *inputDevice,
                                        ulong timestamp,
                                        const QPointF & local,
                                        const QPointF & global,
                                        Qt::MouseButtons b,
                                        Qt::KeyboardModifiers mods);

    static const wl_callback_listener callbackListener;
    static void frameCallback(void *data, struct wl_callback *wl_callback, uint32_t time);

};


#endif // QWAYLANDWINDOW_H
