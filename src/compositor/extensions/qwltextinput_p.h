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

#ifndef QTWAYLAND_QWLTEXTINPUT_P_H
#define QTWAYLAND_QWLTEXTINPUT_P_H

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

#include <QtWaylandCompositor/QWaylandExtension>
#include <QtWaylandCompositor/private/qwayland-server-text.h>

#include <QRect>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandCompositor;

namespace QtWayland {

class InputMethod;

class TextInput : public QWaylandExtensionTemplate<TextInput>, public QtWaylandServer::wl_text_input
{
public:
    explicit TextInput(QWaylandObject *container, QWaylandCompositor *compositor, struct ::wl_client *client, int id);

    QWaylandSurface *focus() const;

    bool inputPanelVisible() const;
    QRect cursorRectangle() const;

    void deactivate(InputMethod *inputMethod);

protected:
    void text_input_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void text_input_activate(Resource *resource, wl_resource *seat, wl_resource *surface) Q_DECL_OVERRIDE;
    void text_input_deactivate(Resource *resource, wl_resource *seat) Q_DECL_OVERRIDE;
    void text_input_show_input_panel(Resource *resource) Q_DECL_OVERRIDE;
    void text_input_hide_input_panel(Resource *resource) Q_DECL_OVERRIDE;
    void text_input_reset(Resource *resource) Q_DECL_OVERRIDE;
    void text_input_commit_state(Resource *resource, uint32_t serial) Q_DECL_OVERRIDE;
    void text_input_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose) Q_DECL_OVERRIDE;
    void text_input_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) Q_DECL_OVERRIDE;
    void text_input_set_preferred_language(Resource *resource, const QString &language) Q_DECL_OVERRIDE;
    void text_input_set_surrounding_text(Resource *resource, const QString &text, uint32_t cursor, uint32_t anchor) Q_DECL_OVERRIDE;
    void text_input_invoke_action(Resource *resource, uint32_t button, uint32_t index) Q_DECL_OVERRIDE;

private:
    QWaylandCompositor *m_compositor;
    QList<InputMethod*> m_activeInputMethods;
    QWaylandSurface *m_focus;

    bool m_inputPanelVisible;
    QRect m_cursorRectangle;

};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLTEXTINPUT_P_H
