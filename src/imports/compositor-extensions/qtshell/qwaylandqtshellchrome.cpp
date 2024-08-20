// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandqtshellchrome.h"
#include "qwaylandqtshellchrome_p.h"
#include "qwaylandqtshell.h"

#include <QtWaylandCompositor/qwaylandquickshellsurfaceitem.h>

QT_BEGIN_NAMESPACE

QPointF QWaylandQtShellChromePrivate::constrainPoint(const QPointF &point) const
{
    float x0 = maximizedRect.left();
    float y0 = maximizedRect.top();
    float x1 = maximizedRect.right();
    float y1 = maximizedRect.bottom();
    return QPoint(qBound(x0, point.x(), x1),
                  qBound(y0, point.y(), y1));
}

void QWaylandQtShellChromePrivate::updateDecorationInteraction(quint8 flags,
                                                               const QQuickHandlerPoint &centroid)
{
    if (shellSurface == nullptr)
        return;

    if (decorationInteraction == quint8(DecorationInteraction::None)) {
        decorationInteraction = flags;
        decorationInteractionPosition = centroid.scenePressPosition();
        decorationInteractionGeometry = shellSurface->windowGeometry();
    }

    if (decorationInteraction != flags)
        return;

    QPointF position = constrainPoint(centroid.scenePosition());
    float dx = position.x() - decorationInteractionPosition.x();
    float dy = position.y() - decorationInteractionPosition.y();

    float minWidth = qMax(0, shellSurface->minimumSize().width());
    float minHeight = qMax(0, shellSurface->minimumSize().height());

    float maxWidth = shellSurface->maximumSize().width();
    float maxHeight = shellSurface->maximumSize().height();

    float minX = maxWidth >= 0.0f
            ? decorationInteractionGeometry.right() - maxWidth
            : -FLT_MAX;
    float minY = maxHeight >= 0.0f
            ? decorationInteractionGeometry.bottom() - maxHeight
            : -FLT_MAX;
    float maxX = maxWidth >= 0
            ? decorationInteractionGeometry.left() + maxWidth
            : FLT_MAX;
    float maxY = maxHeight >= 0.0f
            ? decorationInteractionGeometry.top() + maxHeight
            : FLT_MAX;

    float newLeft = decorationInteractionGeometry.left();
    if (flags & quint8(DecorationInteraction::WestBound)) {
        newLeft = qBound(minX,
                         newLeft + dx,
                         float(decorationInteractionGeometry.right() - minWidth));
    }

    float newTop = decorationInteractionGeometry.top();
    if (flags & quint8(DecorationInteraction::NorthBound)) {
        newTop = qBound(minY,
                        newTop + dy,
                        decorationInteractionGeometry.bottom() + minHeight);
    }

    float newRight = decorationInteractionGeometry.right();
    if (flags & quint8(DecorationInteraction::EastBound)) {
        newRight = qBound(decorationInteractionGeometry.left() + minWidth,
                          newRight + dx,
                          maxX);
    }

    float newBottom = decorationInteractionGeometry.bottom();
    if (flags & quint8(DecorationInteraction::SouthBound)) {
        newBottom = qBound(decorationInteractionGeometry.top() + minHeight,
                           newBottom + dy,
                           maxY);
    }

    shellSurface->requestWindowGeometry(shellSurface->windowState(),
                                        QRect(int(newLeft), int(newTop),
                                              int(newRight - newLeft), int(newBottom - newTop)));
}

/*!
 * \qmltype QtShellChrome
 * \nativetype QWaylandQtShellChrome
 * \inqmlmodule QtWayland.Compositor.QtShell
 * \since 6.3
 * \brief Provides default window manager functionality for use with the \c qt-shell extension.
 *
 * The QtShellChrome is a convenience type that can be used to provide window manager functionality
 * to the interaction with clients over the \c qt-shell
 * \l{Shell Extensions - Qt Wayland Compositor}{shell extension protocol}.
 *
 * Given a ShellSurfaceItem with an associated QtShellSurface, the item will automatically adapt
 * its size to match the surface. It will also provide automatic handling of:
 * \list
 *   \li Window states, such as maximized, minimized and fullscreen.
 *   \li Window activation.
 *   \li Window resizing using with resize handles (if the appropriate properties are set.)
 *   \li Window repositioning using title bar interaction (if the \l titleBar property is set.)
 * \endlist
 *
 * The QtShellChrome is intended to be used together with QtShell and QtShellSurface.
 *
 * \sa {QtShell Compositor}
 */
QWaylandQtShellChrome::QWaylandQtShellChrome(QQuickItem *parent)
    : QQuickItem(*new QWaylandQtShellChromePrivate{}, parent)
{
    init();
}

QWaylandQtShellChrome::QWaylandQtShellChrome(QWaylandQtShellChromePrivate &dd,
                                             QQuickItem *parent)
    : QQuickItem(dd, parent)
{
    init();
}

QWaylandQtShellChrome::~QWaylandQtShellChrome()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shell != nullptr)
        d->shell->unregisterChrome(this);
}

void QWaylandQtShellChrome::init()
{
    connect(this, &QWaylandQtShellChrome::currentWindowStateChanged,
            this, &QWaylandQtShellChrome::windowMetaInfoChanged);

    connect(this, &QWaylandQtShellChrome::currentWindowFlagsChanged,
            this, &QWaylandQtShellChrome::windowMetaInfoChanged);

    connect(this, &QWaylandQtShellChrome::windowMetaInfoChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::leftResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::rightResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::topResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::bottomResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::topLeftResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::bottomLeftResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::topRightResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);

    connect(this, &QWaylandQtShellChrome::bottomRightResizeHandleChanged,
            this, &QWaylandQtShellChrome::updateDecorations);
}

/*!
 * \qmlmethod void QtShellChrome::toggleFullScreen()
 *
 * Toggles between fullscreen and normal window states. This method also clears the minimized
 * or maximized window states if either is set.
 */
void QWaylandQtShellChrome::toggleFullScreen()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return;

    uint newState;
    if ((d->shellSurface->windowState() & Qt::WindowFullScreen) == Qt::WindowFullScreen)
        newState = d->currentState & ~Qt::WindowFullScreen;
    else
        newState = d->currentState | Qt::WindowFullScreen;

    if ((newState & (Qt::WindowMinimized | Qt::WindowMaximized)) != 0)
        newState &= ~(Qt::WindowMinimized | Qt::WindowMaximized);

    setWindowState(newState);
}

/*!
 * \qmlmethod void QtShellChrome::toggleMaximized()
 *
 * Toggles between maximized and normal states. This method also clears the minimized
 * window state if it is set.
 */
void QWaylandQtShellChrome::toggleMaximized()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return;

    uint newState;
    if ((d->shellSurface->windowState() & Qt::WindowMaximized) == Qt::WindowMaximized)
        newState = d->currentState & ~Qt::WindowMaximized;
    else
        newState = d->currentState | Qt::WindowMaximized;

    if ((newState & Qt::WindowMinimized) == Qt::WindowMinimized)
        newState &= ~Qt::WindowMinimized;

    setWindowState(newState);
}

/*!
 * \qmlmethod void QtShellChrome::toggleMinimized()
 *
 * Toggles between minimized and normal states. This method also clears the maximized
 * window state if it is set.
 */
void QWaylandQtShellChrome::toggleMinimized()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return;

    uint newState;
    if ((d->shellSurface->windowState() & Qt::WindowMinimized) == Qt::WindowMinimized)
        newState = d->currentState & ~Qt::WindowMinimized;
    else
        newState = d->currentState | Qt::WindowMinimized;

    if ((newState & Qt::WindowMaximized) == Qt::WindowMaximized)
        newState &= ~Qt::WindowMaximized;

    setWindowState(newState);
}

/*!
 * \qmlproperty ShellSurfaceItem QtShellChrome::shellSurfaceItem
 *
 * This property holds the shell surface item associated with this QtShellChrome. It will
 * in turn manage the \c shellSurface of this item. The \c shellSurface of the item is expected to
 * be of the type QtShellSurface.
 *
 * \qml
 * QtShellChrome {
 *    id: chrome
 *    ShellSurfaceItem {
 *        id: sfi
 *        anchors.fill: parent
 *        moveItem: chrome
 *    }
 *    shellSurfaceItem: sfi
 * }
 * \endqml
 */
void QWaylandQtShellChrome::setShellSurfaceItem(QWaylandQuickShellSurfaceItem *shellSurfaceItem)
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurfaceItem == shellSurfaceItem)
        return;

    if (d->shellSurfaceItem != nullptr)
        d->shellSurfaceItem->disconnect(this);

    d->shellSurfaceItem = shellSurfaceItem;

    if (d->shellSurfaceItem != nullptr) {
        connect(d->shellSurfaceItem, &QWaylandQuickShellSurfaceItem::shellSurfaceChanged,
                this, &QWaylandQtShellChrome::updateShellSurface);
        connect(d->shellSurfaceItem, &QWaylandQuickShellSurfaceItem::surfaceDestroyed,
                this, &QWaylandQtShellChrome::clientDestroyed);
    }

    updateShellSurface();
    emit shellSurfaceItemChanged();
}

QWaylandQuickShellSurfaceItem *QWaylandQtShellChrome::shellSurfaceItem() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->shellSurfaceItem;
}

void QWaylandQtShellChrome::stopGrab()
{
    Q_D(QWaylandQtShellChrome);
    d->decorationInteraction = quint8(QWaylandQtShellChromePrivate::DecorationInteraction::None);
}

void QWaylandQtShellChrome::leftResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->leftResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::WestBound),
                                   d->leftResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::rightResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->rightResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::EastBound),
                                   d->rightResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::topResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->topResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::NorthBound),
                                   d->topResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::bottomResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->bottomResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::SouthBound),
                                   d->bottomResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::topLeftResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->topLeftResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::WestBound)
                                   | quint8(QWaylandQtShellChromePrivate::DecorationInteraction::NorthBound),
                                   d->topLeftResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::topRightResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->topRightResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::EastBound)
                                   | quint8(QWaylandQtShellChromePrivate::DecorationInteraction::NorthBound),
                                   d->topRightResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::bottomLeftResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->bottomLeftResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::WestBound)
                                   | quint8(QWaylandQtShellChromePrivate::DecorationInteraction::SouthBound),
                                   d->bottomLeftResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::bottomRightResize()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->bottomRightResizeHandleHandler->active())
        return;

    d->updateDecorationInteraction(quint8(QWaylandQtShellChromePrivate::DecorationInteraction::EastBound)
                                   | quint8(QWaylandQtShellChromePrivate::DecorationInteraction::SouthBound),
                                   d->bottomRightResizeHandleHandler->centroid());
}

void QWaylandQtShellChrome::titleBarMove()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->titleBarHandler->active())
        return;

    quint8 flags = quint8(QWaylandQtShellChromePrivate::DecorationInteraction::TitleBar);
    QQuickHandlerPoint centroid = d->titleBarHandler->centroid();
    if (d->decorationInteraction == quint8(QWaylandQtShellChromePrivate::DecorationInteraction::None)) {
        d->decorationInteraction = flags;
        d->decorationInteractionPosition = d->shellSurface->windowPosition() - centroid.scenePressPosition();

        activate();
    }

    if (d->decorationInteraction != flags)
        return;

    QPointF position = d->constrainPoint(centroid.scenePosition());
    d->shellSurface->setWindowPosition((position + d->decorationInteractionPosition).toPoint());
}

/*!
 * \qmlproperty Item QtShellChrome::titleBar
 *
 * This property holds the default title bar item of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the title bar which moves the window around on user interaction. In
 * addition, the window will automatically be activated if the title bar is clicked.
 *
 * The title bar will automatically hide and show, depending on the window flags and the
 * window's full screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: tb
 *        anchors.top: parent.top
 *        anchors.right: parent.right
 *        anchors.left: parent.left
 *        height: 50
 *        color: "black"
 *
 *        Text {
 *            color: "white"
 *            anchors.centerIn: parent
 *            text: shellSurfaceItem.shellSurface.windowTitle
 *            font.pixelSize: 25
 *        }
 *    }
 *    titleBar: tb
 * }
 * \endqml
 *
 * \note Unless explicit frame margins are set, the title bar's height will be included in the
 * window's top frame margin.
 */
QQuickItem *QWaylandQtShellChrome::titleBar() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->titleBar;
}

void QWaylandQtShellChrome::setTitleBar(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->titleBar == item)
        return;

    if (d->titleBar != nullptr) {
        d->titleBar->disconnect(this);

        delete d->titleBarHandler;
        d->titleBarHandler = nullptr;
    }

    d->titleBar = item;

    if (d->titleBar != nullptr) {
        connect(d->titleBar, &QQuickItem::heightChanged,
                this, &QWaylandQtShellChrome::updateDecorations);

        d->titleBarHandler = new QQuickDragHandler(d->titleBar);
        d->titleBarHandler->setTarget(nullptr);

        connect(d->titleBarHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->titleBarHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::activateOnGrab);
        connect(d->titleBarHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::titleBarMove);
    }

    emit titleBarChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::leftResizeHandle
 *
 * This property holds the default left resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its left edge.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: lrh
 *        anchors.left: parent.left
 *        anchors.top: parent.top
 *        anchors.bottom: parent.bottom
 *        width: 5
 *        color: "white"
 *    }
 *    leftResizeHandle: lrh
 * }
 * \endqml
 *
 * \note Unless explicit frame margins are set, the handle's width will be included in the
 * window's left frame margin.
 */
QQuickItem *QWaylandQtShellChrome::leftResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->leftResizeHandle;
}

void QWaylandQtShellChrome::setLeftResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->leftResizeHandle == item)
        return;

    if (d->leftResizeHandle != nullptr) {
        d->leftResizeHandle->disconnect(this);

        delete d->leftResizeHandleHandler;
        d->leftResizeHandleHandler = nullptr;
    }

    d->leftResizeHandle = item;

    if (d->leftResizeHandle != nullptr) {
        connect(d->leftResizeHandle, &QQuickItem::widthChanged,
                this, &QWaylandQtShellChrome::updateDecorations);

        d->leftResizeHandleHandler = new QQuickDragHandler(d->leftResizeHandle);
        d->leftResizeHandleHandler->setCursorShape(Qt::SizeHorCursor);
        d->leftResizeHandleHandler->setTarget(nullptr);

        connect(d->leftResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->leftResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::leftResize);
    }

    emit leftResizeHandleChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::rightResizeHandle
 *
 * This property holds the default right resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its right edge.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: rrh
 *        anchors.right: parent.right
 *        anchors.top: parent.top
 *        anchors.bottom: parent.bottom
 *        width: 5
 *        color: "white"
 *    }
 *    rightResizeHandle: rrh
 * }
 * \endqml
 *
 * \note Unless explicit frame margins are set, the handle's width will be included in the
 * window's right frame margin.
 */
QQuickItem *QWaylandQtShellChrome::rightResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->rightResizeHandle;
}

void QWaylandQtShellChrome::setRightResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->rightResizeHandle == item)
        return;

    if (d->rightResizeHandle != nullptr) {
        d->rightResizeHandle->disconnect(this);

        delete d->rightResizeHandleHandler;
        d->rightResizeHandleHandler = nullptr;
    }

    d->rightResizeHandle = item;

    if (d->rightResizeHandle != nullptr) {
        connect(d->rightResizeHandle, &QQuickItem::widthChanged,
                this, &QWaylandQtShellChrome::updateDecorations);

        d->rightResizeHandleHandler = new QQuickDragHandler(d->rightResizeHandle);
        d->rightResizeHandleHandler->setCursorShape(Qt::SizeHorCursor);
        d->rightResizeHandleHandler->setTarget(nullptr);

        connect(d->rightResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->rightResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::rightResize);
    }

    emit rightResizeHandleChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::topResizeHandle
 *
 * This property holds the default top resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its top edge.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: trh
 *        anchors.top: parent.top
 *        anchors.left: parent.left
 *        anchors.right: parent.right
 *        height: 5
 *        color: "white"
 *    }
 *    topResizeHandle: trh
 * }
 * \endqml
 *
 * \note Unless explicit frame margins are set, the handle's height will be included in the
 * window's top frame margin.
 */
QQuickItem *QWaylandQtShellChrome::topResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->topResizeHandle;
}

void QWaylandQtShellChrome::setTopResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->topResizeHandle == item)
        return;

    if (d->topResizeHandle != nullptr) {
        d->topResizeHandle->disconnect(this);

        delete d->topResizeHandleHandler;
        d->topResizeHandleHandler = nullptr;
    }

    d->topResizeHandle = item;

    if (d->topResizeHandle != nullptr) {
        connect(d->topResizeHandle, &QQuickItem::heightChanged,
                this, &QWaylandQtShellChrome::updateDecorations);

        d->topResizeHandleHandler = new QQuickDragHandler(d->topResizeHandle);
        d->topResizeHandleHandler->setCursorShape(Qt::SizeVerCursor);
        d->topResizeHandleHandler->setTarget(nullptr);

        connect(d->topResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->topResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::topResize);
    }

    emit topResizeHandleChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::bottomResizeHandle
 *
 * This property holds the default bottom resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its bottom edge.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: brh
 *        anchors.bottom: parent.bottom
 *        anchors.left: parent.left
 *        anchors.right: parent.right
 *        height: 5
 *        color: "white"
 *    }
 *    bottomResizeHandle: brh
 * }
 * \endqml
 *
 * \note Unless explicit frame margins are set, the handle's height will be included in the
 * window's bottom frame margin.
 */
QQuickItem *QWaylandQtShellChrome::bottomResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->bottomResizeHandle;
}

void QWaylandQtShellChrome::setBottomResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->bottomResizeHandle == item)
        return;

    if (d->bottomResizeHandle != nullptr) {
        d->bottomResizeHandle->disconnect(this);

        delete d->bottomResizeHandleHandler;
        d->bottomResizeHandleHandler = nullptr;
    }

    d->bottomResizeHandle = item;

    if (d->bottomResizeHandle != nullptr) {
        connect(d->bottomResizeHandle, &QQuickItem::heightChanged,
                this, &QWaylandQtShellChrome::updateDecorations);

        d->bottomResizeHandleHandler = new QQuickDragHandler(d->bottomResizeHandle);
        d->bottomResizeHandleHandler->setCursorShape(Qt::SizeVerCursor);
        d->bottomResizeHandleHandler->setTarget(nullptr);

        connect(d->bottomResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->bottomResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::bottomResize);

    }

    emit bottomResizeHandleChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::topLeftResizeHandle
 *
 * This property holds the default top-left resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its top and left edges
 * in equal amounts.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: tlrh
 *        anchors.top: parent.top
 *        anchors.left: parent.left
 *        height: 5
 *        width: 5
 *        color: "white"
 *    }
 *    topLeftResizeHandle: tlrh
 * }
 * \endqml
 */
QQuickItem *QWaylandQtShellChrome::topLeftResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->topLeftResizeHandle;
}

void QWaylandQtShellChrome::setTopLeftResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->topLeftResizeHandle == item)
        return;

    if (d->topLeftResizeHandle != nullptr) {
        delete d->topLeftResizeHandleHandler;
        d->topLeftResizeHandleHandler = nullptr;
    }

    d->topLeftResizeHandle = item;

    if (d->topLeftResizeHandle != nullptr) {
        d->topLeftResizeHandleHandler = new QQuickDragHandler(d->topLeftResizeHandle);
        d->topLeftResizeHandleHandler->setCursorShape(Qt::SizeFDiagCursor);
        d->topLeftResizeHandleHandler->setTarget(nullptr);

        connect(d->topLeftResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->topLeftResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::topLeftResize);
    }

    emit topLeftResizeHandleChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::bottomLeftResizeHandle
 *
 * This property holds the default bottom-left resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its bottom and left edges
 * in equal amounts.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: blrh
 *        anchors.bottom: parent.bottom
 *        anchors.left: parent.left
 *        height: 5
 *        width: 5
 *        color: "white"
 *    }
 *    bottomLeftResizeHandle: blrh
 * }
 * \endqml
 */
QQuickItem *QWaylandQtShellChrome::bottomLeftResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->bottomLeftResizeHandle;
}

void QWaylandQtShellChrome::setBottomLeftResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->bottomLeftResizeHandle == item)
        return;

    if (d->bottomLeftResizeHandle != nullptr) {
        delete d->bottomLeftResizeHandleHandler;
        d->bottomLeftResizeHandleHandler = nullptr;
    }

    d->bottomLeftResizeHandle = item;

    if (d->bottomLeftResizeHandle != nullptr) {
        d->bottomLeftResizeHandleHandler = new QQuickDragHandler(d->bottomLeftResizeHandle);
        d->bottomLeftResizeHandleHandler->setCursorShape(Qt::SizeBDiagCursor);
        d->bottomLeftResizeHandleHandler->setTarget(nullptr);

        connect(d->bottomLeftResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->bottomLeftResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::bottomLeftResize);
    }

    emit bottomLeftResizeHandleChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::topRightResizeHandle
 *
 * This property holds the default top-right resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its top and right edges
 * in equal amounts.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: trrh
 *        anchors.top: parent.top
 *        anchors.right: parent.right
 *        height: 5
 *        width: 5
 *        color: "white"
 *    }
 *    topRightResizeHandle: trrh
 * }
 * \endqml
 */
QQuickItem *QWaylandQtShellChrome::topRightResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->topRightResizeHandle;
}

void QWaylandQtShellChrome::setTopRightResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->topRightResizeHandle == item)
        return;

    if (d->topRightResizeHandle != nullptr) {
        delete d->topRightResizeHandleHandler;
        d->topRightResizeHandleHandler = nullptr;
    }

    d->topRightResizeHandle = item;

    if (d->topRightResizeHandle != nullptr) {
        d->topRightResizeHandleHandler = new QQuickDragHandler(d->topRightResizeHandle);
        d->topRightResizeHandleHandler->setCursorShape(Qt::SizeBDiagCursor);
        d->topRightResizeHandleHandler->setTarget(nullptr);

        connect(d->topRightResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->topRightResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::topRightResize);
    }

    emit topRightResizeHandleChanged();
}

/*!
 * \qmlproperty Item QtShellChrome::bottomRightResizeHandle
 *
 * This property holds the default bottom-right resize handle of the QtShellChrome. If set, a \l DragHandler
 * will be installed on the resize handle which resizes the window by moving its bottom and right edges
 * in equal amounts.
 *
 * The handle will automatically hide and show, depending on the window flags and the window's full
 * screen state.
 *
 * \qml
 * QtShellChrome {
 *    Rectangle {
 *        id: brrh
 *        anchors.bottom: parent.bottom
 *        anchors.right: parent.right
 *        height: 5
 *        width: 5
 *        color: "white"
 *    }
 *    bottomRightResizeHandle: brrh
 * }
 * \endqml
 */
QQuickItem *QWaylandQtShellChrome::bottomRightResizeHandle() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->bottomRightResizeHandle;
}

void QWaylandQtShellChrome::setBottomRightResizeHandle(QQuickItem *item)
{
    Q_D(QWaylandQtShellChrome);
    if (d->bottomRightResizeHandle == item)
        return;

    if (d->bottomRightResizeHandle != nullptr) {
        delete d->bottomRightResizeHandleHandler;
        d->bottomRightResizeHandleHandler = nullptr;
    }

    d->bottomRightResizeHandle = item;

    if (d->bottomRightResizeHandle != nullptr) {
        d->bottomRightResizeHandleHandler = new QQuickDragHandler(d->bottomRightResizeHandle);
        d->bottomRightResizeHandleHandler->setCursorShape(Qt::SizeFDiagCursor);
        d->bottomRightResizeHandleHandler->setTarget(nullptr);

        connect(d->bottomRightResizeHandleHandler, &QQuickPointerHandler::grabChanged,
                this, &QWaylandQtShellChrome::stopGrab);
        connect(d->bottomRightResizeHandleHandler, &QQuickMultiPointHandler::centroidChanged,
                this, &QWaylandQtShellChrome::bottomRightResize);
    }

    emit bottomRightResizeHandleChanged();
}

/*!
 * \qmlproperty rect QtShellChrome::maximizedRect
 *
 * This property holds the are of the WaylandOutput which is available to be filled by the
 * window when it is in maximized state. By default, the window will fill the entire geometry
 * of the WaylandOutput when it is maximized. Changing it can be useful for example when the
 * compositor has other system UI which should not be obscured by maximized applications, such as
 * a task bar.
 */
void QWaylandQtShellChrome::setMaximizedRect(const QRect &rect)
{
    Q_D(QWaylandQtShellChrome);
    if (d->maximizedRect == rect)
        return;

    d->maximizedRect = rect;
    emit maximizedRectChanged();
}

QRect QWaylandQtShellChrome::maximizedRect() const
{
    Q_D(const QWaylandQtShellChrome);
    if (d->maximizedRect.isValid())
        return d->maximizedRect;
    else if (d->shellSurfaceItem != nullptr && d->shellSurfaceItem->output() != nullptr)
        return d->shellSurfaceItem->output()->geometry();

    return QRect{};
}

void QWaylandQtShellChrome::updateDecorations()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return;

    bool decorations = hasDecorations();
    bool titleBarShowing = hasTitleBar();

    QMargins margins;
    if (d->automaticFrameMargins) {
        if (d->leftResizeHandle != nullptr && decorations)
            margins.setLeft(d->leftResizeHandle->width());
        if (d->rightResizeHandle != nullptr && decorations)
            margins.setRight(d->rightResizeHandle->width());
        if (d->bottomResizeHandle != nullptr && decorations)
            margins.setBottom(d->bottomResizeHandle->height());

        margins.setTop((decorations && d->topResizeHandle != nullptr ? d->topResizeHandle->height() : 0)
                    + (titleBarShowing && d->titleBar != nullptr ? d->titleBar->height() : 0));
    } else {
        margins = d->explicitFrameMargins;
    }
    d->shellSurface->setFrameMargins(margins);

    if (d->titleBar != nullptr)
        d->titleBar->setVisible(titleBarShowing);
    if (d->leftResizeHandle != nullptr)
        d->leftResizeHandle->setVisible(decorations);
    if (d->rightResizeHandle != nullptr)
        d->rightResizeHandle->setVisible(decorations);
    if (d->topResizeHandle != nullptr)
        d->topResizeHandle->setVisible(decorations);
    if (d->bottomResizeHandle != nullptr)
        d->bottomResizeHandle->setVisible(decorations);
    if (d->bottomLeftResizeHandle != nullptr)
        d->bottomLeftResizeHandle->setVisible(decorations);
    if (d->topLeftResizeHandle != nullptr)
        d->topLeftResizeHandle->setVisible(decorations);
    if (d->bottomRightResizeHandle != nullptr)
        d->bottomRightResizeHandle->setVisible(decorations);
    if (d->topRightResizeHandle != nullptr)
        d->topRightResizeHandle->setVisible(decorations);

    bool minimizedOrMaximized = (d->currentState & (Qt::WindowMaximized|Qt::WindowMinimized)) != 0;
    if (d->leftResizeHandleHandler != nullptr)
        d->leftResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->rightResizeHandleHandler != nullptr)
        d->rightResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->bottomResizeHandleHandler != nullptr)
        d->bottomResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->topResizeHandleHandler != nullptr)
        d->topResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->bottomLeftResizeHandleHandler != nullptr)
        d->bottomLeftResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->bottomRightResizeHandleHandler != nullptr)
        d->bottomRightResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->topLeftResizeHandleHandler != nullptr)
        d->topLeftResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->topRightResizeHandleHandler != nullptr)
        d->topRightResizeHandleHandler->setEnabled(decorations && !minimizedOrMaximized);
    if (d->titleBarHandler != nullptr)
        d->titleBarHandler->setEnabled(titleBarShowing && !minimizedOrMaximized);
}

void QWaylandQtShellChrome::updateGeometry()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return;

    QRect windowGeometry = d->shellSurface->windowGeometry();

    QPointF position = windowGeometry.topLeft();
    position.rx() -= d->shellSurface->frameMarginLeft();
    position.ry() -= d->shellSurface->frameMarginTop();

    QSizeF size = windowGeometry.size();
    size.rwidth() += d->shellSurface->frameMarginLeft() + d->shellSurface->frameMarginRight();
    size.rheight() += d->shellSurface->frameMarginTop() + d->shellSurface->frameMarginBottom();

    setPosition(position);
    setSize(size);
}

void QWaylandQtShellChrome::updateSurface()
{
    Q_D(QWaylandQtShellChrome);
    QWaylandSurface *surface = d->shellSurface != nullptr ? d->shellSurface->surface() : nullptr;
    if (d->surface == surface)
        return;

    if (d->surface != nullptr)
        d->surface->disconnect(this);

    d->surface = surface;

    if (d->surface != nullptr) {
        connect(d->surface, &QWaylandSurface::hasContentChanged,
                this, &QWaylandQtShellChrome::updateAutomaticPosition);
    }
}

void QWaylandQtShellChrome::updateShellSurface()
{
    Q_D(QWaylandQtShellChrome);
    QWaylandQtShellSurface *sf = d->shellSurfaceItem != nullptr
            ? qobject_cast<QWaylandQtShellSurface *>(d->shellSurfaceItem->shellSurface())
            : nullptr;
    if (d->shellSurface == sf)
        return;

    if (d->shellSurface != nullptr) {
        d->shellSurface->disconnect(this);
        if (d->shell != nullptr)
            d->shell->unregisterChrome(this);
        d->shell = nullptr;
    }

    d->shellSurface = sf;
    if (d->shellSurface != nullptr) {
        d->shell = d->shellSurface->shell();
        if (d->shell != nullptr)
            d->shell->registerChrome(this);

        updateWindowFlags();
        connect(d->shellSurface, &QWaylandQtShellSurface::windowFlagsChanged,
                this, &QWaylandQtShellChrome::updateWindowFlags);
        connect(d->shellSurface, &QWaylandQtShellSurface::windowStateChanged,
                this, &QWaylandQtShellChrome::updateWindowState);
        connect(d->shellSurface, &QWaylandQtShellSurface::frameMarginChanged,
                this, &QWaylandQtShellChrome::updateGeometry);
        connect(d->shellSurface, &QWaylandQtShellSurface::windowGeometryChanged,
                this, &QWaylandQtShellChrome::updateGeometry);
        connect(d->shellSurface, &QWaylandQtShellSurface::raiseRequested,
                this, &QWaylandQtShellChrome::raise);
        connect(d->shellSurface, &QWaylandQtShellSurface::lowerRequested,
                this, &QWaylandQtShellChrome::lower);
        connect(d->shellSurface, &QWaylandQtShellSurface::activeChanged,
                this, &QWaylandQtShellChrome::updateActiveState);
        connect(d->shellSurface, &QWaylandQtShellSurface::surfaceChanged,
                this, &QWaylandQtShellChrome::updateSurface);
    }

    updateDecorations();
    updateSurface();
}

void QWaylandQtShellChrome::updateWindowState()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return;

    setWindowState(d->shellSurface->windowState());
}

void QWaylandQtShellChrome::updateWindowFlags()
{
    Q_D(QWaylandQtShellChrome);

    uint nextFlags = d->shellSurface == nullptr || d->shellSurface->windowFlags() == Qt::Window
            ? d->defaultFlags
            : d->shellSurface->windowFlags();

    if (d->currentFlags != nextFlags) {
        d->currentFlags = nextFlags;
        emit currentWindowFlagsChanged();
    }
}

/*!
 * \qmlproperty int QtShellChrome::windowFlags
 *
 * This property holds the window flags of the QtShellChrome. They will match the \c windowFlags
 * property of the associated QtShellSurface, except when this is equal to Qt.Window. In this case,
 * a set of default window flags will be used instead. The default window flags are Qt.Window,
 * Qt.WindowMaximizeButtonHint, Qt.WindowMinimizeButtonHint and Qt.WindowCloseButtonHint.
 */
uint QWaylandQtShellChrome::currentWindowFlags() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->currentFlags;
}

/*!
 * \qmlproperty int QtShellChrome::windowState
 *
 * This property holds the window state of the shell surface. It will be updated immediately when
 * the window state is requested on the compositor-side, before this has been acknowledged by the
 * client. Therefore, it may in brief periods differ from the shell surface's \c windowState
 * property, which will be updated when the client has acknowledged the request.
 */
uint QWaylandQtShellChrome::currentWindowState() const
{
    Q_D(const QWaylandQtShellChrome);
    return d->currentState;
}

bool QWaylandQtShellChrome::hasTitleBar() const
{
    Q_D(const QWaylandQtShellChrome);

    bool frameless = (d->currentFlags & Qt::FramelessWindowHint) == Qt::FramelessWindowHint
            || ((d->currentFlags & Qt::Popup) == Qt::Popup
                && (d->currentFlags & Qt::Tool) != Qt::Tool)
            || (d->currentState & Qt::WindowFullScreen) == Qt::WindowFullScreen;
    return !frameless;
}

/*!
 * \qmlproperty bool QtShellChrome::hasDecorations
 *
 * This property is true if the QtShellChrome's decorations should be visible, based on its window
 * state and window flags.
 */
bool QWaylandQtShellChrome::hasDecorations() const
{
    Q_D(const QWaylandQtShellChrome);

    return hasTitleBar() && (d->currentFlags & Qt::Window) == Qt::Window;
}

QRect QWaylandQtShellChrome::maxContentRect() const
{
    Q_D(const QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return QRect{};

    int x0 = d->maximizedRect.x() + d->shellSurface->frameMarginLeft();
    int x1 = d->maximizedRect.x() + d->maximizedRect.width() - d->shellSurface->frameMarginRight();
    int y0 = d->maximizedRect.y() + d->shellSurface->frameMarginTop();
    int y1 = d->maximizedRect.y() + d->maximizedRect.height() - d->shellSurface->frameMarginBottom();

    return QRect(x0, y0, x1 - x0, y1 - y0);
}

static int randomPos(int windowSize, int screenSize)
{
    return (windowSize >= screenSize) ? 0 : rand() % (screenSize - windowSize);
}

void QWaylandQtShellChrome::setWindowState(uint nextState)
{
    Q_D(QWaylandQtShellChrome);

    if (d->currentState == nextState)
        return;

    if (d->shellSurface == nullptr || d->shellSurfaceItem == nullptr)
        return;

    QWaylandOutput *output = d->shellSurfaceItem->output();
    if (output == nullptr)
        return;

    if ((d->currentState & (Qt::WindowMinimized | Qt::WindowMaximized | Qt::WindowFullScreen)) == 0) {
        d->restoreGeometry = d->shellSurface->windowGeometry();
    }

    d->currentState = nextState;
    emit currentWindowStateChanged();

    if ((nextState & Qt::WindowMinimized) != 0) {
        d->shellSurface->requestWindowGeometry(nextState, QRect(0, 0, 1, 1));
        d->shellSurfaceItem->setVisible(false);
        deactivate();
    } else if ((nextState & Qt::WindowFullScreen) != 0) {
        d->shellSurfaceItem->setVisible(true);
        d->shellSurface->requestWindowGeometry(nextState, QRect(QPoint(0, 0), output->window()->size()));
        activate();
    } else if ((nextState & Qt::WindowMaximized) != 0) {
        d->shellSurfaceItem->setVisible(true);
        d->shellSurface->requestWindowGeometry(nextState, maxContentRect());
        activate();
    } else {
        d->shellSurfaceItem->setVisible(true);
        d->shellSurface->requestWindowGeometry(nextState, d->restoreGeometry);
        activate();
    }
}

void QWaylandQtShellChrome::updateAutomaticPosition()
{
    Q_D(QWaylandQtShellChrome);
    if (!d->positionSet && d->shellSurface != nullptr) {
        bool randomize = d->shellSurface->positionAutomatic();
        QRect rect = d->shellSurface->windowGeometry();
        QRect space = maxContentRect();

        int xpos = randomize ? randomPos(rect.width(), space.width()) + space.x()
                             : qMax(rect.x(), space.x());
        int ypos = randomize ? randomPos(rect.height(), space.height()) + space.y()
                             : qMax(rect.y(), space.y());

        d->shellSurface->setWindowPosition(QPoint(xpos, ypos));
        d->positionSet = true;
    }
}

/*!
 * \qmlmethod void QtShellChrome::deactivate()
 *
 * Manually deactivates this window. If the window was active, this will activate the next window in
 * the stack instead.
 */
void QWaylandQtShellChrome::deactivate()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface != nullptr)
        d->shellSurface->setActive(false);
}

void QWaylandQtShellChrome::activateOnGrab(QPointingDevice::GrabTransition transition)
{
    Q_D(QWaylandQtShellChrome);
    if (d->titleBarHandler != nullptr) {
        switch (transition) {
        case QPointingDevice::GrabPassive:
        case QPointingDevice::OverrideGrabPassive:
        case QPointingDevice::GrabExclusive:
            activate();
            break;
        default:
            break;
        }
    }
}

/*!
 * \qmlmethod void QtShellChrome::activate()
 *
 * Manually activate this window. This will also raise the window.
 *
 * \sa raise()
 */
void QWaylandQtShellChrome::activate()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface != nullptr)
        d->shellSurface->setActive(true);
    raise();
}

/*!
 * \qmlmethod void QtShellChrome::raise()
 *
 * Raise this window, so that it stacks on top of other windows (except if the other window's
 * flags prohibit this.)
 */
void QWaylandQtShellChrome::raise()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurfaceItem != nullptr)
        d->shellSurfaceItem->raise();
}

/*!
 * \qmlmethod void QtShellChrome::lower()
 *
 * Lower this window, so that it stacks underneath other windows (except if the other window's
 * window flags prohibit this.)
 */
void QWaylandQtShellChrome::lower()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurfaceItem != nullptr)
        d->shellSurfaceItem->lower();
}

void QWaylandQtShellChrome::updateActiveState()
{
    Q_D(QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return;

    if (d->shellSurface->active()) {
        raise();
        emit activated();
    } else {
        emit deactivated();
    }
}

/*!
 * \qmlproperty int QtShellChrome::frameMarginLeft
 *
 * Sets the size of the left margin of the QtShellChrome which is reserved for window decorations.
 * By default, this will equal the width of the \l leftResizeHandle if it is set. Otherwise it will
 * be 0.
 *
 * \note By setting this property explicitly, all default frame margins will be overridden with
 * their corresponding properties.
 */
void QWaylandQtShellChrome::setFrameMarginLeft(int left)
{
    Q_D(QWaylandQtShellChrome);
    if (d->explicitFrameMargins.left() == left)
        return;

    d->explicitFrameMargins.setLeft(left);
    d->automaticFrameMargins = false;
    updateDecorations();

    emit frameMarginChanged();
}

int QWaylandQtShellChrome::frameMarginLeft() const
{
    Q_D(const QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return 0;
    return d->shellSurface->frameMarginLeft();
}

/*!
 * \qmlproperty int QtShellChrome::frameMarginRight
 *
 * Sets the size of the right margin of the QtShellChrome which is reserved for window decorations.
 * By default, this will equal the width of the \l rightResizeHandle if it is set. Otherwise it will
 * be 0.
 *
 * \note By setting this property explicitly, all default frame margins will be overridden with
 * their corresponding properties.
 */
void QWaylandQtShellChrome::setFrameMarginRight(int right)
{
    Q_D(QWaylandQtShellChrome);
    if (d->explicitFrameMargins.right() == right)
        return;

    d->explicitFrameMargins.setRight(right);
    d->automaticFrameMargins = false;
    updateDecorations();

    emit frameMarginChanged();
}

int QWaylandQtShellChrome::frameMarginRight() const
{
    Q_D(const QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return 0;
    return d->shellSurface->frameMarginRight();
}

/*!
 * \qmlproperty int QtShellChrome::frameMarginTop
 *
 * Sets the size of the top margin of the QtShellChrome which is reserved for window decorations.
 * By default, this will equal the sum of the \l leftResizeHandle and the \l{titleBar}'s heights,
 * if they are set. Otherwise it will be 0.
 *
 * \note By setting this property explicitly, all default frame margins will be overridden with
 * their corresponding properties.
 */
void QWaylandQtShellChrome::setFrameMarginTop(int top)
{
    Q_D(QWaylandQtShellChrome);
    if (d->explicitFrameMargins.top() == top)
        return;
    d->explicitFrameMargins.setTop(top);
    d->automaticFrameMargins = false;
    updateDecorations();

    emit frameMarginChanged();
}

int QWaylandQtShellChrome::frameMarginTop() const
{
    Q_D(const QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return 0;
    return d->shellSurface->frameMarginTop();
}

/*!
 * \qmlproperty int QtShellChrome::frameMarginBottom
 *
 * Sets the size of the bottom margin of the QtShellChrome which is reserved for window decorations.
 * By default, this will equal the height of the \l bottomResizeHandle if it is set. Otherwise it will
 * be 0.
 *
 * \note By setting this property explicitly, all default frame margins will be overridden with
 * their corresponding properties.
 */
void QWaylandQtShellChrome::setFrameMarginBottom(int bottom)
{
    Q_D(QWaylandQtShellChrome);
    if (d->explicitFrameMargins.bottom() == bottom)
        return;
    d->explicitFrameMargins.setBottom(bottom);
    d->automaticFrameMargins = false;
    updateDecorations();

    emit frameMarginChanged();
}

int QWaylandQtShellChrome::frameMarginBottom() const
{
    Q_D(const QWaylandQtShellChrome);
    if (d->shellSurface == nullptr)
        return 0;
    return d->shellSurface->frameMarginBottom();
}

QT_END_NAMESPACE

#include "moc_qwaylandqtshellchrome.cpp"
