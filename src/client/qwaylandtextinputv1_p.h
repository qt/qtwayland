/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandClient module of the Qt Toolkit.
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


#ifndef QWAYLANDTEXTINPUTV1_H
#define QWAYLANDTEXTINPUTV1_H

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

#include "qwaylandtextinputinterface_p.h"
#include <QtWaylandClient/private/qwayland-text-input-unstable-v1.h>
#include <qwaylandinputmethodeventbuilder_p.h>

struct wl_callback;
struct wl_callback_listener;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;

class QWaylandTextInputv1 : public QtWayland::zwp_text_input_v1, public QWaylandTextInputInterface
{
public:
    QWaylandTextInputv1(QWaylandDisplay *display, struct ::zwp_text_input_v1 *text_input);
    ~QWaylandTextInputv1() override;

    void setSeat(struct ::wl_seat *seat) { m_seat = seat; }

    void reset() override;
    void commit() override;
    void updateState(Qt::InputMethodQueries queries, uint32_t flags) override;

    void setCursorInsidePreedit(int cursor) override;

    bool isInputPanelVisible() const override;
    QRectF keyboardRect() const override;

    QLocale locale() const override;
    Qt::LayoutDirection inputDirection() const override;

    void showInputPanel() override
    {
        show_input_panel();
    }
    void hideInputPanel() override
    {
        hide_input_panel();
    }
    void enableSurface(::wl_surface *surface) override
    {
        activate(m_seat, surface);
    }
    void disableSurface(::wl_surface *surface) override
    {
        Q_UNUSED(surface);
        deactivate(m_seat);
    }

protected:
    void zwp_text_input_v1_enter(struct ::wl_surface *surface) override;
    void zwp_text_input_v1_leave() override;
    void zwp_text_input_v1_modifiers_map(wl_array *map) override;
    void zwp_text_input_v1_input_panel_state(uint32_t state) override;
    void zwp_text_input_v1_preedit_string(uint32_t serial, const QString &text, const QString &commit) override;
    void zwp_text_input_v1_preedit_styling(uint32_t index, uint32_t length, uint32_t style) override;
    void zwp_text_input_v1_preedit_cursor(int32_t index) override;
    void zwp_text_input_v1_commit_string(uint32_t serial, const QString &text) override;
    void zwp_text_input_v1_cursor_position(int32_t index, int32_t anchor) override;
    void zwp_text_input_v1_delete_surrounding_text(int32_t before_length, uint32_t after_length) override;
    void zwp_text_input_v1_keysym(uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers) override;
    void zwp_text_input_v1_language(uint32_t serial, const QString &language) override;
    void zwp_text_input_v1_text_direction(uint32_t serial, uint32_t direction) override;

private:
    Qt::KeyboardModifiers modifiersToQtModifiers(uint32_t modifiers);

    QWaylandDisplay *m_display = nullptr;
    QWaylandInputMethodEventBuilder m_builder;

    QList<Qt::KeyboardModifier> m_modifiersMap;

    uint32_t m_serial = 0;
    struct ::wl_surface *m_surface = nullptr;
    struct ::wl_seat *m_seat = nullptr;

    QString m_preeditCommit;

    bool m_inputPanelVisible = false;
    QRectF m_keyboardRectangle;
    QLocale m_locale;
    Qt::LayoutDirection m_inputDirection = Qt::LayoutDirectionAuto;

    struct ::wl_callback *m_resetCallback = nullptr;
    static const wl_callback_listener callbackListener;
    static void resetCallback(void *data, struct wl_callback *wl_callback, uint32_t time);
};

}

QT_END_NAMESPACE
#endif // QWAYLANDTEXTINPUTV1_H

