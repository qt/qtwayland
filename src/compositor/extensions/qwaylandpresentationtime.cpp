// Copyright (C) 2021 LG Electronics Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandpresentationtime_p.h"
#include "qwaylandpresentationtime_p_p.h"

#include <time.h>
#include <QQuickWindow>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandQuickItem>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype PresentationTime
 * \instantiates QWaylandPresentationTime
 * \inqmlmodule QtWayland.Compositor.PresentationTime
 * \since 6.3
 * \brief Provides tracking the timing when a frame is presented on screen.
 *
 * The PresentationTime extension provides a way to track rendering timing
 * for a surface. Client can request feedbacks associated with a surface,
 * then compositor send events for the feedback with the time when the surface
 * is presented on-screen.
 *
 * PresentationTime corresponds to the Wayland \c wp_presentation interface.
 *
 * To provide the functionality of the presentationtime extension in a compositor, create
 * an instance of the PresentationTime component and add it to the list of extensions
 * supported by the compositor:
 *
 * Then, call sendFeedback() when a surface is presented on screen.
 * Usually, the timing can be obtained from drm page flip event.
 *
 * \qml
 * import QtWayland.Compositor.PresentationTime
 *
 * WaylandCompositor {
 *     PresentationTime {
 *         id: presentationTime
 *     }
 * }
 * \endqml
 */

/*!
 * \class QWaylandPresentationTime
 * \inmodule QtWaylandCompositor
 * \since 6.3
 * \brief The QWaylandPresentationTime class is an extension to get timing for on-screen presentation.
 *
 * The QWaylandPresentationTime extension provides a way to track rendering timing
 * for a surface. Client can request feedbacks associated with a surface,
 * then compositor send events for the feedback with the time when the surface
 * is presented on-screen.
 *
 * QWaylandPresentationTime corresponds to the Wayland \c wp_presentation interface.
 */


/*!
 * Constructs a QWaylandPresentationTime object for \a compositor.
 */
QWaylandPresentationTime::QWaylandPresentationTime(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(compositor, *new QWaylandPresentationTimePrivate)
{

}

/*!
 * Constructs an empty QWaylandPresentationTime object.
 */
QWaylandPresentationTime::QWaylandPresentationTime()
    : QWaylandCompositorExtensionTemplate(*new QWaylandPresentationTimePrivate)
{
}

/*!
 * Initializes the extension.
 */
void QWaylandPresentationTime::initialize()
{
    Q_D(QWaylandPresentationTime);

    if (isInitialized()) {
        qWarning() << "QWaylandPresentationTime is already initialized";
        return;
    }

    QWaylandCompositor *compositor = this->compositor();
    if (compositor == nullptr) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandPresentationTime";
        return;
    }

    QWaylandCompositorExtensionTemplate::initialize();

    d->init(compositor->display(), /* version */ 1);
}

QWaylandCompositor *QWaylandPresentationTime::compositor() const
{
    return qobject_cast<QWaylandCompositor *>(extensionContainer());
}

/*!
 * \qmlmethod void PresentationTime::sendFeedback(Window window, int sequence, int sec, int nsec)
 *
 * Interface to notify that a frame is presented on screen using \a window.
 * If your platform supports DRM events, \c page_flip_handler is the proper timing to send it.
 * The \a sequence is the refresh counter. \a sec and \a nsec hold the
 * seconds and nanoseconds parts of the presentation timestamp, respectively.
 */

/*!
 * Interface to notify that a frame is presented on screen using \a window.
 * If your platform supports DRM events, \c page_flip_handler is the proper timing to send it.
 * The \a sequence is the refresh counter. \a tv_sec and \a tv_nsec hold the
 * seconds and nanoseconds parts of the presentation timestamp, respectively.
 */
void QWaylandPresentationTime::sendFeedback(QQuickWindow *window, quint64 sequence, quint64 tv_sec, quint32 tv_nsec)
{
    if (!window)
        return;

    quint32 refresh_nsec = window->screen()->refreshRate() != 0 ? 1000000000 / window->screen()->refreshRate() : 0;

    emit presented(sequence, tv_sec, tv_nsec, refresh_nsec);
}

/*!
 * Returns the Wayland interface for the QWaylandPresentationTime.
 */
const struct wl_interface *QWaylandPresentationTime::interface()
{
    return QWaylandPresentationTimePrivate::interface();
}

/*!
 * \internal
 */
QByteArray QWaylandPresentationTime::interfaceName()
{
    return QWaylandPresentationTimePrivate::interfaceName();
}

PresentationFeedback::PresentationFeedback(QWaylandPresentationTime *pTime, QWaylandSurface *surface, struct ::wl_client *client, uint32_t id, int version)
    : wp_presentation_feedback(client, id, version)
    , m_presentationTime(pTime)
{
    setSurface(surface);
}

void PresentationFeedback::setSurface(QWaylandSurface *qwls)
{
    if (!qwls) {
        discard();
        return;
    }

    m_surface = qwls;

    connect(qwls, &QWaylandSurface::damaged, this, &PresentationFeedback::onSurfaceCommit);
    connect(qwls, &QWaylandSurface::destroyed, this, &PresentationFeedback::discard);

    QWaylandView *view = qwls ? qwls->primaryView() : nullptr;
    //The surface has not committed yet.
    if (!view) {
        connect(qwls, &QWaylandSurface::hasContentChanged, this, &PresentationFeedback::onSurfaceMapped);
        return;
    }

    maybeConnectToWindow(view);
}

void PresentationFeedback::onSurfaceCommit()
{
    // There is a new commit before sync so that discard this feedback.
    if (m_committed) {
        discard();
        return;
    }
    m_committed = true;
}

void PresentationFeedback::onSurfaceMapped()
{
    QWaylandView *view = m_surface->primaryView();
    if (!view) {
        qWarning() << "The mapped surface has no view";
        discard();
        return;
    }

    maybeConnectToWindow(view);
}

void PresentationFeedback::maybeConnectToWindow(QWaylandView *view)
{
    QWaylandQuickItem *item = view ? qobject_cast<QWaylandQuickItem *>(view->renderObject()) : nullptr;
    if (!item) {
        qWarning() << "QWaylandPresentationTime only works with QtQuick compositors" << view;
        discard();
        return;
    }

    connect(item, &QQuickItem::windowChanged, this, &PresentationFeedback::onWindowChanged);
    // wait for having window
    if (!item->window()) {
       return;
    }

    connectToWindow(item->window());
}

void PresentationFeedback::onWindowChanged()
{
    QWaylandQuickItem *item = qobject_cast<QWaylandQuickItem *>(sender());
    QQuickWindow *window = item ? item->window() : nullptr;

    if (!window) {
        qWarning() << "QWaylandPresentationTime only works with QtQuick compositors" << item;
        discard();
        /* Actually, the commit is not discarded yet. If the related item has new window,
           the commit can be presented on screen. So we can choose not to discard the feedback
           until item has new window or the surface is destroyed. */
        return;
    }

    // Check if the connected window is changed
    if (m_connectedWindow && m_connectedWindow != window)
        m_connectedWindow->disconnect(this);

    connectToWindow(window);
}

void PresentationFeedback::connectToWindow(QQuickWindow *window)
{
    if (!window) {
        discard();
        return;
    }

    m_connectedWindow = window;

    connect(window, &QQuickWindow::beforeSynchronizing, this, &PresentationFeedback::onSync);
    connect(window, &QQuickWindow::afterFrameEnd, this, &PresentationFeedback::onSwapped);
}

void PresentationFeedback::onSync()
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(sender());

    if (m_committed) {
        disconnect(m_surface, &QWaylandSurface::damaged, this, &PresentationFeedback::onSurfaceCommit);
        disconnect(window, &QQuickWindow::beforeSynchronizing, this, &PresentationFeedback::onSync);
        m_sync = true;
    }
}

void PresentationFeedback::onSwapped()
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(sender());

    if (m_sync) {
        disconnect(window, &QQuickWindow::afterFrameEnd, this, &PresentationFeedback::onSwapped);
        connect(m_presentationTime, &QWaylandPresentationTime::presented, this, &PresentationFeedback::sendPresented);
    }
}

void PresentationFeedback::discard()
{
    send_discarded();
    destroy();
}

void PresentationFeedback::sendSyncOutput()
{
    QWaylandCompositor *compositor = presentationTime()->compositor();
    if (!compositor) {
        qWarning() << "No compositor container to send sync_output";
        return;
    }

    QWaylandView *view = surface()->primaryView();
    QWaylandOutput *output = view ? view->output() : nullptr;
    struct ::wl_resource *r = output ? output->resourceForClient(QWaylandClient::fromWlClient(compositor, resource()->client())) : nullptr;

    if (r)
        send_sync_output(r);
}

void PresentationFeedback::sendPresented(quint64 sequence, quint64 tv_sec, quint32 tv_nsec, quint32 refresh_nsec)
{
    sendSyncOutput();

    send_presented(tv_sec >> 32, tv_sec, tv_nsec, refresh_nsec, sequence >> 32, sequence,
            QtWaylandServer::wp_presentation_feedback::kind_vsync
            | QtWaylandServer::wp_presentation_feedback::kind_hw_clock
            | QtWaylandServer::wp_presentation_feedback::kind_hw_completion);

    destroy();
}

void PresentationFeedback::destroy()
{
    wl_resource_destroy(resource()->handle);
}

void PresentationFeedback::wp_presentation_feedback_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

QWaylandPresentationTimePrivate::QWaylandPresentationTimePrivate()
{
}

void QWaylandPresentationTimePrivate::wp_presentation_bind_resource(Resource *resource)
{
    send_clock_id(resource->handle, CLOCK_MONOTONIC);
}

void QWaylandPresentationTimePrivate::wp_presentation_feedback(Resource *resource, struct ::wl_resource *surface, uint32_t callback)
{
    Q_Q(QWaylandPresentationTime);

    QWaylandSurface *qwls = QWaylandSurface::fromResource(surface);
    if (!qwls)
        return;

    new PresentationFeedback(q, qwls, resource->client(), callback, /* version */ 1);
}

QT_END_NAMESPACE

#include "moc_qwaylandpresentationtime_p_p.cpp"

#include "moc_qwaylandpresentationtime_p.cpp"
