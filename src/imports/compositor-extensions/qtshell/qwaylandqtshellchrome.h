// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQTSHELLCHROME_H
#define QWAYLANDQTSHELLCHROME_H

#include <QtQuick/qquickitem.h>
#include <QtWaylandCompositor/qwaylandquickshellsurfaceitem.h>

QT_BEGIN_NAMESPACE

class QWaylandQtShellChromePrivate;
class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtShellChrome : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(QtShellChrome)
    QML_ADDED_IN_VERSION(1, 0)
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
