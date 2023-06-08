// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandquickshellsurfaceitem.h"
#include "qwaylandquickshellsurfaceitem_p.h"

#include <QtWaylandCompositor/QWaylandShellSurface>
#include <QGuiApplication>

QT_BEGIN_NAMESPACE

QWaylandQuickShellSurfaceItem *QWaylandQuickShellSurfaceItemPrivate::maybeCreateAutoPopup(QWaylandShellSurface* shellSurface)
{
    if (!m_autoCreatePopupItems)
        return nullptr;

    Q_Q(QWaylandQuickShellSurfaceItem);
    auto *popupItem = new QWaylandQuickShellSurfaceItem(q);
    popupItem->setShellSurface(shellSurface);
    popupItem->setAutoCreatePopupItems(true);
    QObject::connect(popupItem, &QWaylandQuickShellSurfaceItem::surfaceDestroyed, [popupItem](){
        popupItem->deleteLater();
    });
    return popupItem;
}

/*!
 * \qmltype ShellSurfaceItem
 * \instantiates QWaylandQuickShellSurfaceItem
 * \inherits WaylandQuickItem
 * \inqmlmodule QtWayland.Compositor
 * \since 5.8
 * \brief A Qt Quick item type for displaying and interacting with a ShellSurface.
 *
 * This type is used to render \c wl_shell, \c xdg_shell or \c ivi_application surfaces as part of
 * a Qt Quick scene. It handles moving and resizing triggered by clicking on the window decorations.
 *
 * \sa WaylandQuickItem, WlShellSurface, IviSurface
 */

/*!
 * \class QWaylandQuickShellSurfaceItem
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The QWaylandQuickShellSurfaceItem class provides a Qt Quick item that represents a QWaylandShellSurface.
 *
 * This class is used to render \c wl_shell, \c xdg_shell or \c ivi_application surfaces as part of
 * a Qt Quick scene. It handles moving and resizing triggered by clicking on the window decorations.
 *
 * \sa QWaylandQuickItem, QWaylandWlShellSurface, QWaylandIviSurface
 */

/*!
 * Constructs a QWaylandQuickWlShellSurfaceItem with the given \a parent.
 */
QWaylandQuickShellSurfaceItem::QWaylandQuickShellSurfaceItem(QQuickItem *parent)
    : QWaylandQuickItem(*new QWaylandQuickShellSurfaceItemPrivate(), parent)
{
}

QWaylandQuickShellSurfaceItem::~QWaylandQuickShellSurfaceItem()
{
    Q_D(QWaylandQuickShellSurfaceItem);

    if (d->m_shellIntegration) {
        removeEventFilter(d->m_shellIntegration);
        delete d->m_shellIntegration;
    }
}

/*!
 * \internal
 */
QWaylandQuickShellSurfaceItem::QWaylandQuickShellSurfaceItem(QWaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent)
    : QWaylandQuickItem(dd, parent)
{
}

/*!
 * \qmlproperty ShellSurface QtWayland.Compositor::ShellSurfaceItem::shellSurface
 *
 * This property holds the ShellSurface rendered by this ShellSurfaceItem.
 * It may either be an XdgSurfaceV5, WlShellSurface or IviSurface depending on which shell protocol
 * is in use.
 */

/*!
 * \property QWaylandQuickShellSurfaceItem::shellSurface
 *
 * This property holds the QWaylandShellSurface rendered by this QWaylandQuickShellSurfaceItem.
 * It may either be a QWaylandXdgSurfaceV5, QWaylandWlShellSurface or QWaylandIviSurface depending
 * on which shell protocol is in use.
 */
QWaylandShellSurface *QWaylandQuickShellSurfaceItem::shellSurface() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->m_shellSurface;
}

void QWaylandQuickShellSurfaceItem::setShellSurface(QWaylandShellSurface *shellSurface)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (d->m_shellSurface == shellSurface)
        return;

    d->m_shellSurface = shellSurface;

    if (d->m_shellIntegration) {
        removeEventFilter(d->m_shellIntegration);
        delete d->m_shellIntegration;
        d->m_shellIntegration = nullptr;
    }

    if (shellSurface) {
        d->m_shellIntegration = shellSurface->createIntegration(this);
        installEventFilter(d->m_shellIntegration);
    }

    emit shellSurfaceChanged();
}

/*!
 * \qmlproperty Item QtWayland.Compositor::ShellSurfaceItem::moveItem
 *
 * This property holds the move item for this ShellSurfaceItem. This is the item that will be moved
 * when the clients request the ShellSurface to be moved, maximized, resized etc. This property is
 * useful when implementing server-side decorations.
 */

/*!
 * \property QWaylandQuickShellSurfaceItem::moveItem
 *
 * This property holds the move item for this QWaylandQuickShellSurfaceItem. This is the item that
 * will be moved when the clients request the QWaylandShellSurface to be moved, maximized, resized
 * etc. This property is useful when implementing server-side decorations.
 */
QQuickItem *QWaylandQuickShellSurfaceItem::moveItem() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->m_moveItem ? d->m_moveItem : const_cast<QWaylandQuickShellSurfaceItem *>(this);
}

void QWaylandQuickShellSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    moveItem = moveItem ? moveItem : this;
    if (this->moveItem() == moveItem)
        return;
    d->m_moveItem = moveItem;
    moveItemChanged();
}

/*!
 * \qmlproperty bool QtWayland.Compositor::ShellSurfaceItem::autoCreatePopupItems
 *
 * This property holds whether ShellSurfaceItems for popups parented to the shell
 * surface managed by this item should automatically be created.
 */

/*!
 * \property QWaylandQuickShellSurfaceItem::autoCreatePopupItems
 *
 * This property holds whether QWaylandQuickShellSurfaceItems for popups
 * parented to the shell surface managed by this item should automatically be created.
 */
bool QWaylandQuickShellSurfaceItem::autoCreatePopupItems()
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->m_autoCreatePopupItems;
}

void QWaylandQuickShellSurfaceItem::setAutoCreatePopupItems(bool enabled)
{
    Q_D(QWaylandQuickShellSurfaceItem);

    if (enabled == d->m_autoCreatePopupItems)
        return;

    d->m_autoCreatePopupItems = enabled;
    emit autoCreatePopupItemsChanged();
}

/*!
\class QWaylandQuickShellEventFilter
\brief QWaylandQuickShellEventFilter implements a Wayland popup grab
\internal
*/

void QWaylandQuickShellEventFilter::startFilter(QWaylandClient *client, CallbackFunction closePopups)
{
    if (!self)
        self = new QWaylandQuickShellEventFilter(qGuiApp);
    if (!self->eventFilterInstalled) {
        qGuiApp->installEventFilter(self);
        self->eventFilterInstalled = true;
        self->client = client;
        self->closePopups = closePopups;
    }
}

void QWaylandQuickShellEventFilter::cancelFilter()
{
    if (!self)
        return;
    if (self->eventFilterInstalled && !self->waitForRelease)
        self->stopFilter();
}

void QWaylandQuickShellEventFilter::stopFilter()
{
    if (eventFilterInstalled) {
        qGuiApp->removeEventFilter(this);
        eventFilterInstalled = false;
    }
}
QWaylandQuickShellEventFilter *QWaylandQuickShellEventFilter::self = nullptr;

QWaylandQuickShellEventFilter::QWaylandQuickShellEventFilter(QObject *parent)
    : QObject(parent)
{
}

bool QWaylandQuickShellEventFilter::eventFilter(QObject *receiver, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
        bool press = e->type() == QEvent::MouseButtonPress;
        if (press && !waitForRelease) {
            // The user clicked something: we need to close popups unless this press is caught later
            if (!mousePressTimeout.isActive())
                mousePressTimeout.start(0, this);
        }

        QQuickItem *item = qobject_cast<QQuickItem*>(receiver);
        if (!item)
            return false;

        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        QWaylandQuickShellSurfaceItem *shellSurfaceItem = qobject_cast<QWaylandQuickShellSurfaceItem*>(item);
        bool finalRelease = (event->type() == QEvent::MouseButtonRelease) && (event->buttons() == Qt::NoButton);
        bool popupClient = shellSurfaceItem && shellSurfaceItem->surface() && shellSurfaceItem->surface()->client() == client;

        if (waitForRelease) {
            // We are eating events until all mouse buttons are released
            if (finalRelease) {
                waitForRelease = false;
                stopFilter();
            }
            return true;
        }

        if (finalRelease && mousePressTimeout.isActive()) {
            // the user somehow managed to press and release the mouse button in 0 milliseconds
            qWarning("Badly written autotest detected");
            mousePressTimeout.stop();
            stopFilter();
        }

        if (press && !shellSurfaceItem && !QQmlProperty(item, QStringLiteral("qtwayland_blocking_overlay")).isValid()) {
            // the user clicked on something that's not blocking mouse events
            e->ignore(); //propagate the event to items below
            return true; // don't give the event to the item
        }

        mousePressTimeout.stop(); // we've got this

        if (press && !popupClient) {
            // The user clicked outside the active popup's client. The popups should
            // be closed, but the event filter will stay to catch the release-
            // event before removing itself.
            waitForRelease = true;
            closePopups();
            return true;
        }
    }

    return false;
}

void QWaylandQuickShellEventFilter::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mousePressTimeout.timerId()) {
        mousePressTimeout.stop();
        closePopups();
        stopFilter();
        // Don't wait for release: Since the press wasn't accepted,
        // the release won't be delivered.
    }
}

static QWaylandQuickShellSurfaceItem *findSurfaceItemFromMoveItem(QQuickItem *moveItem)
{
    if (Q_UNLIKELY(!moveItem))
        return nullptr;
    if (auto *surf = qobject_cast<QWaylandQuickShellSurfaceItem *>(moveItem))
        return surf;
    for (auto *item : moveItem->childItems()) {
        if (auto *surf = findSurfaceItemFromMoveItem(item))
            return surf;
    }
    return nullptr;
}

/*
    To raise a surface, find the topmost suitable surface and place above that.
    We start from the top and:
    If we don't have staysOnTop, skip all surfaces with staysOnTop
    If we have staysOnBottom, skip all surfaces that don't have staysOnBottom
  */
void QWaylandQuickShellSurfaceItemPrivate::raise()
{
    Q_Q(QWaylandQuickShellSurfaceItem);
    auto *moveItem = q->moveItem();
    QQuickItem *parent = moveItem->parentItem();
    if (!parent)
        return;
    auto it = parent->childItems().crbegin();
    auto skip = [this](QQuickItem *item) {
        if (auto *surf = findSurfaceItemFromMoveItem(item))
            return (!staysOnTop && surf->staysOnTop()) || (staysOnBottom && !surf->staysOnBottom());
        return true; // ignore any other Quick items that may be there
    };
    auto end = parent->childItems().crend();
    while (it != end && skip(*it))
        ++it;
    if (it != end) {
        QQuickItem *top = *it;
        if (moveItem != top)
            moveItem->stackAfter(top);
    }
}

/*
    To lower a surface, find the lowest suitable surface and place below that.
    We start from the bottom and:
    If we don't have staysOnBottom, skip all surfaces with staysOnBottom
    If we have staysOnTop, skip all surfaces that don't have staysOnTop
  */
void QWaylandQuickShellSurfaceItemPrivate::lower()
{
    Q_Q(QWaylandQuickShellSurfaceItem);
    auto *moveItem = q->moveItem();
    QQuickItem *parent = moveItem->parentItem();
    if (!parent)
        return;
    auto it = parent->childItems().cbegin();

    auto skip = [this](QQuickItem *item) {
        if (auto *surf = findSurfaceItemFromMoveItem(item))
            return (!staysOnBottom && surf->staysOnBottom()) || (staysOnTop && !surf->staysOnTop());
        return true; // ignore any other Quick items that may be there
    };
    while (skip(*it))
        ++it;

    QQuickItem *bottom = *it;
    if (moveItem != bottom)
        moveItem->stackBefore(bottom);
}

/*!
 * \property QWaylandQuickShellSurfaceItem::staysOnTop
 *
 * Keep this item above other Wayland surfaces
 */
bool QWaylandQuickShellSurfaceItem::staysOnTop() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->staysOnTop;
}

void QWaylandQuickShellSurfaceItem::setStaysOnTop(bool onTop)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (d->staysOnTop == onTop)
        return;
    d->staysOnTop = onTop;
    if (d->staysOnBottom) {
        d->staysOnBottom = false;
        emit staysOnBottomChanged();
    }
    // We need to call raise() even if onTop is false, since we need to stack under any other
    // staysOnTop surfaces in that case
    raise();
    emit staysOnTopChanged();
    Q_ASSERT(!(d->staysOnTop && d->staysOnBottom));
}

/*!
 * \property QWaylandQuickShellSurfaceItem::staysOnBottom
 *
 * Keep this item above other Wayland surfaces
 */
bool QWaylandQuickShellSurfaceItem::staysOnBottom() const
{
    Q_D(const QWaylandQuickShellSurfaceItem);
    return d->staysOnBottom;
}

void QWaylandQuickShellSurfaceItem::setStaysOnBottom(bool onBottom)
{
    Q_D(QWaylandQuickShellSurfaceItem);
    if (d->staysOnBottom == onBottom)
        return;
    d->staysOnBottom = onBottom;
    if (d->staysOnTop) {
        d->staysOnTop = false;
        emit staysOnTopChanged();
    }
    // We need to call lower() even if onBottom is false, since we need to stack over any other
    // staysOnBottom surfaces in that case
    lower();
    emit staysOnBottomChanged();
    Q_ASSERT(!(d->staysOnTop && d->staysOnBottom));
}

QT_END_NAMESPACE

#include "moc_qwaylandquickshellsurfaceitem_p.cpp"

#include "moc_qwaylandquickshellsurfaceitem.cpp"
