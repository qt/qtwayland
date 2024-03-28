// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTV3_P_H
#define QWAYLANDTEXTINPUTV3_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-text-input-unstable-v3.h>
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

class QWaylandTextInputV3ClientState {
public:
    QWaylandTextInputV3ClientState();

    Qt::InputMethodQueries updatedQueries(const QWaylandTextInputV3ClientState &other) const;
    Qt::InputMethodQueries mergeChanged(const QWaylandTextInputV3ClientState &other);

    Qt::InputMethodHints hints = Qt::ImhNone;
    QRect cursorRectangle;
    QString surroundingText;
    int cursorPosition = 0;
    int anchorPosition = 0;

    Qt::InputMethodQueries changedState;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputV3Private : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::zwp_text_input_v3
{
    Q_DECLARE_PUBLIC(QWaylandTextInputV3)
public:
    explicit QWaylandTextInputV3Private(QWaylandCompositor *compositor);

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

    QScopedPointer<QWaylandTextInputV3ClientState> currentState;
    QScopedPointer<QWaylandTextInputV3ClientState> pendingState;

    QHash<Resource *, uint32_t> serials;
    QHash<Resource *, QWaylandSurface *> enabledSurfaces;

protected:
    void zwp_text_input_v3_bind_resource(Resource *resource) override;
    void zwp_text_input_v3_destroy_resource(Resource *resource) override;

    void zwp_text_input_v3_destroy(Resource *resource) override;
    void zwp_text_input_v3_enable(Resource *resource) override;
    void zwp_text_input_v3_disable(Resource *resource) override;
    void zwp_text_input_v3_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor) override;
    void zwp_text_input_v3_set_text_change_cause(Resource *resource, uint32_t cause) override;
    void zwp_text_input_v3_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose) override;
    void zwp_text_input_v3_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zwp_text_input_v3_commit(Resource *resource) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTV3_P_H
