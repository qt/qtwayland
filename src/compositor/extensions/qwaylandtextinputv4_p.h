/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDTEXTINPUTV4_P_H
#define QWAYLANDTEXTINPUTV4_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-text-input-unstable-v4-wip.h>
#include <QtWaylandCompositor/QWaylandDestroyListener>

#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtGui/QInputMethod>
#include <QtWaylandCompositor/QWaylandSurface>

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

QT_BEGIN_NAMESPACE

class QInputMethodEvent;
class QKeyEvent;
class QWaylandCompositor;
class QWaylandView;

class QWaylandTextInputV4ClientState {
public:
    QWaylandTextInputV4ClientState();

    Qt::InputMethodQueries updatedQueries(const QWaylandTextInputV4ClientState &other) const;
    Qt::InputMethodQueries mergeChanged(const QWaylandTextInputV4ClientState &other);

    Qt::InputMethodHints hints = Qt::ImhNone;
    QRect cursorRectangle;
    QString surroundingText;
    int cursorPosition = 0;
    int anchorPosition = 0;

    Qt::InputMethodQueries changedState;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputV4Private : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::zwp_text_input_v4
{
    Q_DECLARE_PUBLIC(QWaylandTextInputV4)
public:
    explicit QWaylandTextInputV4Private(QWaylandCompositor *compositor);

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    void setFocus(QWaylandSurface *surface);

    QWaylandCompositor *compositor = nullptr;

    QWaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    QWaylandDestroyListener focusDestroyListener;

    bool inputPanelVisible = false;

    QString currentPreeditString;

    QScopedPointer<QWaylandTextInputV4ClientState> currentState;
    QScopedPointer<QWaylandTextInputV4ClientState> pendingState;

    uint32_t serial = 0;

    QHash<Resource *, QWaylandSurface*> enabledSurfaces;

protected:
    void zwp_text_input_v4_bind_resource(Resource *resource) override;
    void zwp_text_input_v4_destroy_resource(Resource *resource) override;

    void zwp_text_input_v4_destroy(Resource *resource) override;
    void zwp_text_input_v4_enable(Resource *resource) override;
    void zwp_text_input_v4_disable(Resource *resource) override;
    void zwp_text_input_v4_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor) override;
    void zwp_text_input_v4_set_text_change_cause(Resource *resource, uint32_t cause) override;
    void zwp_text_input_v4_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose) override;
    void zwp_text_input_v4_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zwp_text_input_v4_commit(Resource *resource) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTV4_P_H
