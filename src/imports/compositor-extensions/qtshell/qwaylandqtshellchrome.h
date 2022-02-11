/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDQTSHELLCHROME_H
#define QWAYLANDQTSHELLCHROME_H

#include <QtQuick/qquickitem.h>
#include <QtWaylandCompositor/qwaylandquickshellsurfaceitem.h>

QT_BEGIN_NAMESPACE

class QWaylandQtShellChromePrivate;
class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtShellChrome : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQtShellChrome)
    Q_PROPERTY(bool hasDecorations READ hasDecorations NOTIFY windowMetaInfoChanged)
    Q_PROPERTY(uint windowState READ currentWindowState NOTIFY currentWindowStateChanged)
    Q_PROPERTY(uint windowFlags READ currentWindowFlags NOTIFY currentWindowFlagsChanged)
    Q_PROPERTY(QWaylandQuickShellSurfaceItem *shellSurfaceItem READ shellSurfaceItem WRITE setShellSurfaceItem NOTIFY shellSurfaceItemChanged)
    Q_PROPERTY(QRect maximizedRect READ maximizedRect WRITE setMaximizedRect NOTIFY maximizedRectChanged)

    Q_PROPERTY(int frameMarginLeft READ frameMarginLeft WRITE setFrameMarginLeft NOTIFY frameMarginChanged)
    Q_PROPERTY(int frameMarginRight READ frameMarginRight WRITE setFrameMarginRight NOTIFY frameMarginChanged)
    Q_PROPERTY(int frameMarginTop READ frameMarginTop WRITE setFrameMarginTop NOTIFY frameMarginChanged)
    Q_PROPERTY(int frameMarginBottom READ frameMarginBottom WRITE setFrameMarginBottom NOTIFY frameMarginChanged)

    Q_PROPERTY(QQuickItem *titleBar READ titleBar WRITE setTitleBar NOTIFY titleBarChanged);
    Q_PROPERTY(QQuickItem *leftResizeHandle READ leftResizeHandle WRITE setLeftResizeHandle NOTIFY leftResizeHandleChanged);
    Q_PROPERTY(QQuickItem *rightResizeHandle READ rightResizeHandle WRITE setRightResizeHandle NOTIFY rightResizeHandleChanged);
    Q_PROPERTY(QQuickItem *topResizeHandle READ topResizeHandle WRITE setTopResizeHandle NOTIFY topResizeHandleChanged);
    Q_PROPERTY(QQuickItem *bottomResizeHandle READ bottomResizeHandle WRITE setBottomResizeHandle NOTIFY bottomResizeHandleChanged);
    Q_PROPERTY(QQuickItem *topLeftResizeHandle READ topLeftResizeHandle WRITE setTopLeftResizeHandle NOTIFY topLeftResizeHandleChanged);
    Q_PROPERTY(QQuickItem *topRightResizeHandle READ topRightResizeHandle WRITE setTopRightResizeHandle NOTIFY topRightResizeHandleChanged);
    Q_PROPERTY(QQuickItem *bottomLeftResizeHandle READ bottomLeftResizeHandle WRITE setBottomLeftResizeHandle NOTIFY bottomLeftResizeHandleChanged);
    Q_PROPERTY(QQuickItem *bottomRightResizeHandle READ bottomRightResizeHandle WRITE setBottomRightResizeHandle NOTIFY bottomRightResizeHandleChanged);
public:
    QWaylandQtShellChrome(QQuickItem *parent = nullptr);
    ~QWaylandQtShellChrome() override;

    bool hasTitleBar() const;
    bool hasDecorations() const;
    uint currentWindowState() const;
    uint currentWindowFlags() const;

    void setMaximizedRect(const QRect &rect);
    QRect maximizedRect() const;

    void setShellSurfaceItem(QWaylandQuickShellSurfaceItem *shellSurfaceItem);
    QWaylandQuickShellSurfaceItem *shellSurfaceItem() const;

    void setTitleBar(QQuickItem *item);
    QQuickItem *titleBar() const;

    void setLeftResizeHandle(QQuickItem *item);
    QQuickItem *leftResizeHandle() const;

    void setRightResizeHandle(QQuickItem *item);
    QQuickItem *rightResizeHandle() const;

    void setTopResizeHandle(QQuickItem *item);
    QQuickItem *topResizeHandle() const;

    void setBottomResizeHandle(QQuickItem *item);
    QQuickItem *bottomResizeHandle() const;

    void setTopLeftResizeHandle(QQuickItem *item);
    QQuickItem *topLeftResizeHandle() const;

    void setBottomLeftResizeHandle(QQuickItem *item);
    QQuickItem *bottomLeftResizeHandle() const;

    void setTopRightResizeHandle(QQuickItem *item);
    QQuickItem *topRightResizeHandle() const;

    void setBottomRightResizeHandle(QQuickItem *item);
    QQuickItem *bottomRightResizeHandle() const;

    int frameMarginLeft() const;
    void setFrameMarginLeft(int left);

    int frameMarginRight() const;
    void setFrameMarginRight(int right);

    int frameMarginTop() const;
    void setFrameMarginTop(int top);

    int frameMarginBottom() const;
    void setFrameMarginBottom(int bottom);

Q_SIGNALS:
    void currentWindowStateChanged();
    void currentWindowFlagsChanged();
    void windowMetaInfoChanged();
    void shellSurfaceItemChanged();
    void maximizedRectChanged();

    void titleBarChanged();
    void leftResizeHandleChanged();
    void rightResizeHandleChanged();
    void topResizeHandleChanged();
    void bottomResizeHandleChanged();
    void topLeftResizeHandleChanged();
    void bottomLeftResizeHandleChanged();
    void topRightResizeHandleChanged();
    void bottomRightResizeHandleChanged();

    void activated();
    void deactivated();

    void clientDestroyed();
    void frameMarginChanged();

public Q_SLOTS:
    void raise();
    void lower();
    void toggleMaximized();
    void toggleMinimized();
    void toggleFullScreen();
    void activate();
    void deactivate();

private Q_SLOTS:
    void activateOnGrab(QPointingDevice::GrabTransition transition);
    void updateSurface();
    void updateShellSurface();
    void updateWindowFlags();
    void updateWindowState();
    void updateGeometry();
    void updateDecorations();
    void updateActiveState();
    void updateAutomaticPosition();
    void stopGrab();
    void leftResize();
    void rightResize();
    void topResize();
    void bottomResize();
    void topLeftResize();
    void topRightResize();
    void bottomLeftResize();
    void bottomRightResize();
    void titleBarMove();

protected:
    QWaylandQtShellChrome(QWaylandQtShellChromePrivate &dd, QQuickItem *parent);

private:
    void setWindowState(uint nextState);
    void init();
    QRect maxContentRect() const;
};

QT_END_NAMESPACE

#endif // QWAYLANDQTSHELLSURFACEITEM_H
