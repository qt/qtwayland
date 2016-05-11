/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef QWAYLANDWLSHELLINTEGRATION_H
#define QWAYLANDWLSHELLINTEGRATION_H

#include <QtWaylandCompositor/private/qwaylandquickshellsurfaceitem_p.h>

#include <QtWaylandCompositor/QWaylandWlShellSurface>

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace QtWayland {

class WlShellIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    WlShellIntegration(QWaylandQuickShellSurfaceItem *item);
    bool mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void handleStartMove(QWaylandInputDevice *inputDevice);
    void handleStartResize(QWaylandInputDevice *inputDevice, QWaylandWlShellSurface::ResizeEdge edges);
    void handleSetPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parent, const QPoint &relativeToParent);
    void handleShellSurfaceDestroyed();
    void handleSurfaceUnmapped();
    void adjustOffsetForNextFrame(const QPointF &offset);

private:
    enum class GrabberState {
        Default,
        Resize,
        Move
    };

    bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;

    void setIsPopup(bool popup);
    void setFilterEnabled(bool enabled);
    static void closePopups();

    QWaylandQuickShellSurfaceItem *m_item;
    QWaylandWlShellSurface *m_shellSurface;
    GrabberState grabberState;
    struct {
        QWaylandInputDevice *inputDevice;
        QPointF initialOffset;
        bool initialized;
    } moveState;
    struct {
        QWaylandInputDevice *inputDevice;
        QWaylandWlShellSurface::ResizeEdge resizeEdges;
        QSizeF initialSize;
        QPointF initialMousePos;
        bool initialized;
    } resizeState;

    static QVector<QWaylandWlShellSurface*> popupShellSurfaces;
    static bool eventFilterInstalled;
    static bool waitForRelease;
    bool isPopup;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDWLSHELLINTEGRATION_H
