// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandquickoutput.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandquickitem_p.h"

QT_BEGIN_NAMESPACE

QWaylandQuickOutput::QWaylandQuickOutput()
{
}

QWaylandQuickOutput::QWaylandQuickOutput(QWaylandCompositor *compositor, QWindow *window)
    : QWaylandOutput(compositor, window)
{
}

void QWaylandQuickOutput::initialize()
{
    QWaylandOutput::initialize();

    QQuickWindow *quickWindow = qobject_cast<QQuickWindow *>(window());
    if (!quickWindow) {
        qWarning("Initialization error: Could not locate QQuickWindow on initializing QWaylandQuickOutput %p.\n", this);
        return;
    }
    connect(quickWindow, &QQuickWindow::beforeSynchronizing,
            this, &QWaylandQuickOutput::updateStarted,
            Qt::DirectConnection);

    connect(quickWindow, &QQuickWindow::afterRendering,
            this, &QWaylandQuickOutput::doFrameCallbacks);
}

void QWaylandQuickOutput::classBegin()
{
}

void QWaylandQuickOutput::componentComplete()
{
    if (!compositor()) {
        for (QObject *p = parent(); p != nullptr; p = p->parent()) {
            if (auto c = qobject_cast<QWaylandCompositor *>(p)) {
                setCompositor(c);
                break;
            }
        }
    }
}

void QWaylandQuickOutput::update()
{
    if (!m_updateScheduled) {
        //don't qobject_cast since we have verified the type in initialize
        static_cast<QQuickWindow *>(window())->update();
        m_updateScheduled = true;
    }
}

/*!
 * \qmlproperty bool QtWayland.Compositor::WaylandOutput::automaticFrameCallback
 *
 * This property holds whether the WaylandOutput automatically sends frame
 * callbacks when rendering.
 *
 * The default is true.
 */
bool QWaylandQuickOutput::automaticFrameCallback() const
{
    return m_automaticFrameCallback;
}

void QWaylandQuickOutput::setAutomaticFrameCallback(bool automatic)
{
    if (m_automaticFrameCallback == automatic)
        return;

    m_automaticFrameCallback = automatic;
    automaticFrameCallbackChanged();
}

static QQuickItem* clickableItemAtPosition(QQuickItem *rootItem, const QPointF &position)
{
    if (!rootItem->isEnabled() || !rootItem->isVisible())
        return nullptr;

    QList<QQuickItem *> paintOrderItems = QQuickItemPrivate::get(rootItem)->paintOrderChildItems();
    auto negativeZStart = paintOrderItems.crend();
    for (auto it = paintOrderItems.crbegin(); it != paintOrderItems.crend(); ++it) {
        if ((*it)->z() < 0) {
            negativeZStart = it;
            break;
        }
        QQuickItem *item = clickableItemAtPosition(*it, rootItem->mapToItem(*it, position));
        if (item)
            return item;
    }

    if (rootItem->contains(position) && rootItem->acceptedMouseButtons() != Qt::NoButton)
        return rootItem;

    for (auto it = negativeZStart; it != paintOrderItems.crend(); ++it) {
        QQuickItem *item = clickableItemAtPosition(*it, rootItem->mapToItem(*it, position));
        if (item)
            return item;
    }

    return nullptr;
}

QQuickItem *QWaylandQuickOutput::pickClickableItem(const QPointF &position)
{
    QQuickWindow *quickWindow = qobject_cast<QQuickWindow *>(window());
    if (!quickWindow)
        return nullptr;

    return clickableItemAtPosition(quickWindow->contentItem(), position);
}

/*!
 * \internal
 */
void QWaylandQuickOutput::updateStarted()
{
    m_updateScheduled = false;

    if (!compositor())
        return;

    frameStarted();
}

void QWaylandQuickOutput::doFrameCallbacks()
{
    if (m_automaticFrameCallback)
        sendFrameCallbacks();
}
QT_END_NAMESPACE

#include "moc_qwaylandquickoutput.cpp"
