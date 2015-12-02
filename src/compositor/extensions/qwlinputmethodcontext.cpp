/****************************************************************************
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
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

#include "qwlinputmethodcontext_p.h"

#include "qwltextinput_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

InputMethodContext::InputMethodContext(wl_client *client, TextInput *textInput)
    : QtWaylandServer::wl_input_method_context(client, 0, 1)
    , m_textInput(textInput)
{
}

InputMethodContext::~InputMethodContext()
{
}

void InputMethodContext::input_method_context_destroy_resource(Resource *)
{
    delete this;
}

void InputMethodContext::input_method_context_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void InputMethodContext::input_method_context_commit_string(Resource *, uint32_t serial, const QString &text)
{
    m_textInput->send_commit_string(serial, text);
}

void InputMethodContext::input_method_context_cursor_position(Resource *, int32_t index, int32_t anchor)
{
    m_textInput->send_cursor_position(index, anchor);
}

void InputMethodContext::input_method_context_delete_surrounding_text(Resource *, int32_t index, uint32_t length)
{
    m_textInput->send_delete_surrounding_text(index, length);
}

void InputMethodContext::input_method_context_language(Resource *, uint32_t serial, const QString &language)
{
    m_textInput->send_language(serial, language);
}

void InputMethodContext::input_method_context_keysym(Resource *, uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers)
{
    m_textInput->send_keysym(serial, time, sym, state, modifiers);
}

void InputMethodContext::input_method_context_modifiers_map(Resource *, wl_array *map)
{
    QByteArray modifiersArray(static_cast<char *>(map->data), map->size);
    m_textInput->send_modifiers_map(modifiersArray);
}

void InputMethodContext::input_method_context_preedit_cursor(Resource *, int32_t index)
{
    m_textInput->send_preedit_cursor(index);
}

void InputMethodContext::input_method_context_preedit_string(Resource *, uint32_t serial, const QString &text, const QString &commit)
{
    m_textInput->send_preedit_string(serial, text, commit);
}

void InputMethodContext::input_method_context_preedit_styling(Resource *, uint32_t index, uint32_t length, uint32_t style)
{
    m_textInput->send_preedit_styling(index, length, style);
}

void InputMethodContext::input_method_context_grab_keyboard(Resource *, uint32_t keyboard)
{
    Q_UNUSED(keyboard);
}

void InputMethodContext::input_method_context_key(Resource *, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(serial);
    Q_UNUSED(time);
    Q_UNUSED(key);
    Q_UNUSED(state);
}

void InputMethodContext::input_method_context_modifiers(Resource *, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    Q_UNUSED(serial);
    Q_UNUSED(mods_depressed);
    Q_UNUSED(mods_latched);
    Q_UNUSED(mods_locked);
    Q_UNUSED(group);
}

} // namespace QtWayland

QT_END_NAMESPACE
