// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUT_P_H
#define QWAYLANDTEXTINPUT_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-text-input-unstable-v2.h>
#include <QtWaylandCompositor/QWaylandDestroyListener>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QHash>
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

class QWaylandTextInputClientState {
public:
    QWaylandTextInputClientState();

    Qt::InputMethodQueries updatedQueries(const QWaylandTextInputClientState &other) const;
    Qt::InputMethodQueries mergeChanged(const QWaylandTextInputClientState &other);

    Qt::InputMethodHints hints = Qt::ImhNone;
    QRect cursorRectangle;
    QString surroundingText;
    int cursorPosition = 0;
    int anchorPosition = 0;
    QString preferredLanguage;

    Qt::InputMethodQueries changedState;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputPrivate : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::zwp_text_input_v2
{
    Q_DECLARE_PUBLIC(QWaylandTextInput)
public:
    explicit QWaylandTextInputPrivate(QWaylandCompositor *compositor);

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);
    void sendInputPanelState();
    void sendTextDirection();
    void sendLocale();
    void sendModifiersMap(const QByteArray &modifiersMap);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    void setFocus(QWaylandSurface *surface);

    QWaylandCompositor *compositor = nullptr;

    QWaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    QWaylandDestroyListener focusDestroyListener;

    bool inputPanelVisible = false;

    std::unique_ptr<QWaylandTextInputClientState> currentState;
    std::unique_ptr<QWaylandTextInputClientState> pendingState;

    uint32_t serial = 0;

    QHash<Resource *, QWaylandSurface*> enabledSurfaces;

protected:
    void zwp_text_input_v2_bind_resource(Resource *resource) override;
    void zwp_text_input_v2_destroy_resource(Resource *resource) override;

    void zwp_text_input_v2_destroy(Resource *resource) override;
    void zwp_text_input_v2_enable(Resource *resource, wl_resource *surface) override;
    void zwp_text_input_v2_disable(Resource *resource, wl_resource *surface) override;
    void zwp_text_input_v2_show_input_panel(Resource *resource) override;
    void zwp_text_input_v2_hide_input_panel(Resource *resource) override;
    void zwp_text_input_v2_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor) override;
    void zwp_text_input_v2_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose) override;
    void zwp_text_input_v2_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zwp_text_input_v2_set_preferred_language(Resource *resource, const QString &language) override;
    void zwp_text_input_v2_update_state(Resource *resource, uint32_t serial, uint32_t flags) override;

private:
    quint32 shiftModifierMask = 1;
    quint32 controlModifierMask = 2;
    quint32 altModifierMask = 4;
    quint32 metaModifierMask = 8;
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUT_P_H
