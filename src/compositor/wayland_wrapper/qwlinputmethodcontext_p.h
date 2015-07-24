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

#ifndef QTWAYLAND_QWLINPUTMETHODCONTEXT_P_H
#define QTWAYLAND_QWLINPUTMETHODCONTEXT_P_H

#include <QtCompositor/private/qwayland-server-input-method.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class TextInput;

class InputMethodContext : public QtWaylandServer::wl_input_method_context
{
public:
    explicit InputMethodContext(struct ::wl_client *client, TextInput *textInput);
    ~InputMethodContext();

protected:
    void input_method_context_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void input_method_context_destroy(Resource *resource) Q_DECL_OVERRIDE;

    void input_method_context_commit_string(Resource *resource, uint32_t serial, const QString &text) Q_DECL_OVERRIDE;
    void input_method_context_cursor_position(Resource *resource, int32_t index, int32_t anchor) Q_DECL_OVERRIDE;
    void input_method_context_delete_surrounding_text(Resource *resource, int32_t index, uint32_t length) Q_DECL_OVERRIDE;
    void input_method_context_language(Resource *resource, uint32_t serial, const QString &language) Q_DECL_OVERRIDE;
    void input_method_context_keysym(Resource *resource, uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers) Q_DECL_OVERRIDE;
    void input_method_context_modifiers_map(Resource *resource, wl_array *map) Q_DECL_OVERRIDE;
    void input_method_context_preedit_cursor(Resource *resource, int32_t index) Q_DECL_OVERRIDE;
    void input_method_context_preedit_string(Resource *resource, uint32_t serial, const QString &text, const QString &commit) Q_DECL_OVERRIDE;
    void input_method_context_preedit_styling(Resource *resource, uint32_t index, uint32_t length, uint32_t style) Q_DECL_OVERRIDE;
    void input_method_context_grab_keyboard(Resource *resource, uint32_t keyboard) Q_DECL_OVERRIDE;
    void input_method_context_key(Resource *resource, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) Q_DECL_OVERRIDE;
    void input_method_context_modifiers(Resource *resource, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) Q_DECL_OVERRIDE;

private:
    TextInput *m_textInput;
};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLINPUTMETHODCONTEXT_P_H
