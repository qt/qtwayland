/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#include "qwaylandview.h"
#include "qwaylandview_p.h"
#include "qwaylandsurface.h"
#include <QtWaylandCompositor/QWaylandInputDevice>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtWaylandCompositor/private/qwaylandsurface_p.h>
#include <QtWaylandCompositor/private/qwaylandoutput_p.h>

#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

void QWaylandViewPrivate::markSurfaceAsDestroyed(QWaylandSurface *surface)
{
    Q_Q(QWaylandView);
    Q_ASSERT(surface == this->surface);

    q->setSurface(Q_NULLPTR);
    emit q->surfaceDestroyed();
}

/*!
 * \qmltype WaylandView
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \brief Represents a view of a surface on an output.
 *
 * The WaylandView corresponds to the presentation of a surface on a specific output, managing
 * the buffers that contain the contents to be rendered. You can have several views into the same surface.
 */

/*!
 * \class QWaylandView
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief Represents a view of a surface on an output.
 *
 * The WaylandView corresponds to the presentation of a surface on a specific output, managing
 * the buffers that contain the contents to be rendered. You can have several views into the same surface.
 */

/*!
 * Constructs a QWaylandView with the given \a renderObject and \a parent.
 */
QWaylandView::QWaylandView(QObject *renderObject, QObject *parent)
    : QObject(*new QWaylandViewPrivate(),parent)
{
    d_func()->renderObject = renderObject;
}

/*!
 * Destroys the QWaylandView.
 */
QWaylandView::~QWaylandView()
{
    Q_D(QWaylandView);
    if (d->surface) {
        if (d->output)
            QWaylandOutputPrivate::get(d->output)->removeView(this, d->surface);
        QWaylandInputDevice *i = d->surface->compositor()->defaultInputDevice();
        if (i->mouseFocus() == this)
            i->setMouseFocus(Q_NULLPTR);

        QWaylandSurfacePrivate::get(d->surface)->derefView(this);
    }

}

/*!
  \internal Didn't we decide to remove this property?
*/
QObject *QWaylandView::renderObject() const
{
    Q_D(const QWaylandView);
    return d->renderObject;
}

/*!
 * \qmlproperty object QtWaylandCompositor::WaylandView::surface
 *
 * This property holds the surface viewed by this WaylandView.
 */

/*!
 * \property QWaylandView::surface
 *
 * This property holds the surface viewed by this QWaylandView.
 */
QWaylandSurface *QWaylandView::surface() const
{
    Q_D(const QWaylandView);
    return d->surface;
}

void QWaylandView::setSurface(QWaylandSurface *newSurface)
{
    Q_D(QWaylandView);
    if (d->surface == newSurface)
        return;


    if (d->surface) {
        QWaylandSurfacePrivate::get(d->surface)->derefView(this);
        if (d->output)
            QWaylandOutputPrivate::get(d->output)->removeView(this, d->surface);
    }

    d->surface = newSurface;

    if (!d->bufferLock) {
        d->currentBuffer = QWaylandBufferRef();
        d->currentDamage = QRegion();
    }

    d->nextBuffer = QWaylandBufferRef();
    d->nextDamage = QRegion();

    if (d->surface) {
        QWaylandSurfacePrivate::get(d->surface)->refView(this);
        if (d->output)
            QWaylandOutputPrivate::get(d->output)->addView(this, d->surface);
    }

    emit surfaceChanged();

}

/*!
 * \qmlproperty object QtWaylandCompositor::WaylandView::surface
 *
 * This property holds the output on which this view displays its surface.
 */

/*!
 * \property QWaylandView::output
 *
 * This property holds the output on which this view displays its surface.
 */
QWaylandOutput *QWaylandView::output() const
{
    Q_D(const QWaylandView);
    return d->output;
}

void QWaylandView::setOutput(QWaylandOutput *newOutput)
{
    Q_D(QWaylandView);
    if (d->output == newOutput)
        return;

    if (d->output && d->surface)
        QWaylandOutputPrivate::get(d->output)->removeView(this, d->surface);

    d->output = newOutput;

    if (d->output && d->surface)
        QWaylandOutputPrivate::get(d->output)->addView(this, d->surface);

    emit outputChanged();
}

/*!
 * Attaches a new buffer \a ref and \a damage region to this QWaylandView. These
 * will become current the next time advance() is called.
 */
void QWaylandView::attach(const QWaylandBufferRef &ref, const QRegion &damage)
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    d->nextBuffer = ref;
    d->nextDamage = damage;
}

/*!
 * Sets the next buffer and damage to current and returns true. If the buffer
 * is locked or if no new buffer has been attached since the last call to
 * advance(), the function returns false and does nothing.
 *
 * If this view is set as its surface's throttling view, discardCurrentBuffer()
 * will be called on all views of the same surface for which the
 * \l{QWaylandView::discardFrontBuffers}{discardFrontBuffers}
 * property is set to true and the current buffer is the same as the
 * throttling view's current buffer.
 *
 * This allows for a design where a primary
 * view can make sure that views running on a lower frequency will release their
 * front buffer references so that the buffer can be reused on the client side,
 * avoiding the situation where the lower frequency views throttle the frame rate
 * of the client application.
 */
bool QWaylandView::advance()
{
    Q_D(QWaylandView);
    if (d->currentBuffer == d->nextBuffer && !d->forceAdvanceSucceed)
        return false;

    if (d->bufferLock)
        return false;

    if (d->surface && d->surface->throttlingView() == this) {
        Q_FOREACH (QWaylandView *view, d->surface->views()) {
            if (view != this && view->discardFrontBuffers() && view->d_func()->currentBuffer == d->currentBuffer)
                view->discardCurrentBuffer();
        }
    }

    QMutexLocker locker(&d->bufferMutex);
    d->forceAdvanceSucceed = false;
    d->currentBuffer = d->nextBuffer;
    d->currentDamage = d->nextDamage;
    return true;
}

/*!
 * Force the view to discard its current buffer, to allow it to be reused on the client side.
 */
void QWaylandView::discardCurrentBuffer()
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    d->currentBuffer = QWaylandBufferRef();
    d->forceAdvanceSucceed = true;
}

/*!
 * Returns a reference to this view's current buffer.
 */
QWaylandBufferRef QWaylandView::currentBuffer()
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    return d->currentBuffer;
}

/*!
 * Returns the current damage region of this view.
 */
QRegion QWaylandView::currentDamage()
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    return d->currentDamage;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandView::bufferLock
 *
 * This property holds whether the view's buffer is currently locked. When
 * the buffer is locked, advance() will not advance to the next buffer,
 * but will instead return false.
 *
 * The default is false.
 */

/*!
 * \property QWaylandView::bufferLock
 *
 * This property holds whether the view's buffer is currently locked. When
 * the buffer is locked, advance() will not advance to the next buffer,
 * but will instead return false.
 *
 * The default is false.
 */
bool QWaylandView::isBufferLocked() const
{
    Q_D(const QWaylandView);
    return d->bufferLock;
}

void QWaylandView::setBufferLock(bool locked)
{
    Q_D(QWaylandView);
    d->bufferLock = locked;
}

/*!
 * \property bool QWaylandView::discardFrontBuffers
 *
 * By default, the view locks the current buffer until advance() is called. Set this property
 * to true to allow Qt to release the buffer when the throttling view is no longer using it.
 */
bool QWaylandView::discardFrontBuffers() const
{
    Q_D(const QWaylandView);
    return d->discardFrontBuffers;
}

void QWaylandView::setDiscardFrontBuffers(bool discard)
{
    Q_D(QWaylandView);
    if (d->discardFrontBuffers == discard)
        return;
    d->discardFrontBuffers = discard;
    emit discardFrontBuffersChanged();
}

/*!
 * Returns the Wayland surface resource for this QWaylandView.
 */
struct wl_resource *QWaylandView::surfaceResource() const
{
    Q_D(const QWaylandView);
    if (!d->surface)
        return Q_NULLPTR;
    return d->surface->resource();
}

QT_END_NAMESPACE
