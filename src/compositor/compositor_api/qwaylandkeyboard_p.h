/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QTWAYLAND_QWLKEYBOARD_P_H
#define QTWAYLAND_QWLKEYBOARD_P_H

#include <QtWaylandCompositor/qwaylandexport.h>
#include <QtWaylandCompositor/qwaylandinput.h>
#include <QtWaylandCompositor/qwaylandkeyboard.h>
#include <QtWaylandCompositor/qwaylanddestroylistener.h>

#include <QtCore/private/qobject_p.h>
#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QVector>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#endif


QT_BEGIN_NAMESPACE

class Q_COMPOSITOR_EXPORT QWaylandKeyboardPrivate : public QObjectPrivate
                                                  , public QtWaylandServer::wl_keyboard
{
public:
    Q_DECLARE_PUBLIC(QWaylandKeyboard)

    static QWaylandKeyboardPrivate *get(QWaylandKeyboard *keyboard);

    QWaylandKeyboardPrivate(QWaylandInputDevice *seat);
    ~QWaylandKeyboardPrivate();

    QWaylandCompositor *compositor() const { return seat->compositor(); }

    void focused(QWaylandSurface* surface);
    void modifiers(uint32_t serial, uint32_t mods_depressed,
                   uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

#ifndef QT_NO_WAYLAND_XKB
    struct xkb_state *xkbState() const { return xkb_state; }
    uint32_t xkbModsMask() const { return modsDepressed | modsLatched | modsLocked; }
#endif

    void keyEvent(uint code, uint32_t state);
    void sendKeyEvent(uint code, uint32_t state);
    void updateModifierState(uint code, uint32_t state);
    void updateKeymap();

protected:
    void keyboard_bind_resource(Resource *resource);
    void keyboard_destroy_resource(Resource *resource);
    void keyboard_release(Resource *resource) Q_DECL_OVERRIDE;

private:
#ifndef QT_NO_WAYLAND_XKB
    void initXKB();
    void createXKBKeymap();
#endif

    QWaylandInputDevice *seat;

    QWaylandSurface *focus;
    Resource *focusResource;
    QWaylandDestroyListener focusDestroyListener;

    QVector<uint32_t> keys;
    uint32_t modsDepressed;
    uint32_t modsLatched;
    uint32_t modsLocked;
    uint32_t group;

    QWaylandKeymap keymap;
    bool pendingKeymap;
#ifndef QT_NO_WAYLAND_XKB
    size_t keymap_size;
    int keymap_fd;
    char *keymap_area;
    struct xkb_context *xkb_context;
    struct xkb_state *xkb_state;
#endif
};

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLKEYBOARD_P_H
