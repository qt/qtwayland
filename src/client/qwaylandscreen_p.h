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

#ifndef QWAYLANDSCREEN_H
#define QWAYLANDSCREEN_H

#include <qpa/qplatformscreen.h>
#include <QtWaylandClient/private/qwaylandclientexport_p.h>

#include <QtWaylandClient/private/qwayland-wayland.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandCursor;
class QWaylandExtendedOutput;

class Q_WAYLAND_CLIENT_EXPORT QWaylandScreen : public QPlatformScreen, QtWayland::wl_output
{
public:
    QWaylandScreen(QWaylandDisplay *waylandDisplay, int version, uint32_t id);
    ~QWaylandScreen();

    void init();
    QWaylandDisplay *display() const;

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;

    QSizeF physicalSize() const Q_DECL_OVERRIDE;

    QDpi logicalDpi() const Q_DECL_OVERRIDE;
    QList<QPlatformScreen *> virtualSiblings() const Q_DECL_OVERRIDE;

    void setOrientationUpdateMask(Qt::ScreenOrientations mask);

    Qt::ScreenOrientation orientation() const;
    int scale() const;
    qreal devicePixelRatio() const Q_DECL_OVERRIDE;
    qreal refreshRate() const;

    QString name() const { return mOutputName; }

    QPlatformCursor *cursor() const;
    QWaylandCursor *waylandCursor() const { return mWaylandCursor; };

    uint32_t outputId() const { return m_outputId; }
    ::wl_output *output() { return object(); }

    QWaylandExtendedOutput *extendedOutput() const;
    void createExtendedOutput();

    static QWaylandScreen *waylandScreenFromWindow(QWindow *window);

private:
    void output_mode(uint32_t flags, int width, int height, int refresh) Q_DECL_OVERRIDE;
    void output_geometry(int32_t x, int32_t y,
                         int32_t width, int32_t height,
                         int subpixel,
                         const QString &make,
                         const QString &model,
                         int32_t transform) Q_DECL_OVERRIDE;
    void output_scale(int32_t factor) Q_DECL_OVERRIDE;
    void output_done() Q_DECL_OVERRIDE;

    int m_outputId;
    QWaylandDisplay *mWaylandDisplay;
    QWaylandExtendedOutput *mExtendedOutput;
    QRect mGeometry;
    int mScale;
    int mDepth;
    int mRefreshRate;
    int mTransform;
    QImage::Format mFormat;
    QSize mPhysicalSize;
    QString mOutputName;
    Qt::ScreenOrientation m_orientation;

    QWaylandCursor *mWaylandCursor;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDSCREEN_H
