/****************************************************************************
**
** Copyright (C) 2021 LG Electronics Inc.
** Contact: http://www.qt.io/licensing/
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

#ifndef QWAYLANDPRESENTATIONTIME_P_P_H
#define QWAYLANDPRESENTATIONTIME_P_P_H

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

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-presentation-time.h>

#include <QObject>
#include <QPointer>
#include <QMultiMap>

QT_BEGIN_NAMESPACE


class QWaylandSurface;
class QWaylandView;
class QQuickWindow;

class PresentationFeedback : public QObject, public QtWaylandServer::wp_presentation_feedback
{
    Q_OBJECT
public:
    PresentationFeedback(QWaylandPresentationTime *, QWaylandSurface *, struct ::wl_client *, uint32_t, int);

    void setSurface(QWaylandSurface *);
    QWaylandSurface *surface() { return m_surface; }

    void destroy();
    void sendSyncOutput();

private Q_SLOTS:
    void discard();
    void onSurfaceCommit();
    void onSurfaceMapped();
    void onWindowChanged();
    void onSync();
    void onSwapped();
    void sendPresented(quint64 sequence, quint64 tv_sec, quint32 tv_nsec, quint32 refresh_nsec);

private:
    QWaylandPresentationTime *presentationTime() { return m_presentationTime; }
    void maybeConnectToWindow(QWaylandView *);
    void connectToWindow(QQuickWindow *);

    void wp_presentation_feedback_destroy_resource(Resource *resource) override;

public:
    QWaylandPresentationTime *m_presentationTime = nullptr;
    QWaylandSurface *m_surface = nullptr;
    QQuickWindow *m_connectedWindow = nullptr;

    bool m_committed = false;
    bool m_sync = false;
};

class QWaylandPresentationTimePrivate : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::wp_presentation
{
    Q_DECLARE_PUBLIC(QWaylandPresentationTime)
public:
    QWaylandPresentationTimePrivate();

protected:
    void wp_presentation_feedback(Resource *resource, struct ::wl_resource *surface, uint32_t callback) override;
    void wp_presentation_bind_resource(Resource *resource) override;
};

QT_END_NAMESPACE

#endif
