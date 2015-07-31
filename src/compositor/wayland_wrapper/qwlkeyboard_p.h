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

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandinput.h>
#include <QtCompositor/qwaylandkeyboard.h>
#include <QtCompositor/qwaylanddestroylistener.h>

#include <QtCore/private/qobject_p.h>
#include <QtCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QVector>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#endif


QT_BEGIN_NAMESPACE

namespace QtWayland {

class Compositor;
class InputDevice;
class Surface;
class Keyboard;

}

class Q_COMPOSITOR_EXPORT QWaylandKeyboardPrivate : public QObjectPrivate
                                                  , public QtWaylandServer::wl_keyboard
                                                  , public QWaylandKeyboardGrabber
{
public:
    Q_DECLARE_PUBLIC(QWaylandKeyboard)

    QWaylandKeyboardPrivate(QWaylandInputDevice *seat);
    ~QWaylandKeyboardPrivate();

    QWaylandCompositor *compositor() const { return m_seat->compositor(); }
    bool setFocus(QWaylandSurface *surface);
    void setKeymap(const QWaylandKeymap &keymap);

    void sendKeyModifiers(Resource *resource, uint32_t serial);
    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    QWaylandSurface *focus() const;
    Resource *focusResource() const { return m_focusResource; }

    void focused(QWaylandSurface* surface);
    void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void modifiers(uint32_t serial, uint32_t mods_depressed,
                   uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

    void startGrab(QWaylandKeyboardGrabber *grab);
    void endGrab();
    QWaylandKeyboardGrabber *currentGrab() const;

    static QWaylandKeyboardPrivate *get(QWaylandKeyboard *keyboard);

#ifndef QT_NO_WAYLAND_XKB
    struct xkb_state *xkbState() const { return m_state; }
    uint32_t xkbModsMask() const { return m_modsDepressed | m_modsLatched | m_modsLocked; }
#endif
protected:
    void keyboard_bind_resource(Resource *resource);
    void keyboard_destroy_resource(Resource *resource);
    void keyboard_release(Resource *resource) Q_DECL_OVERRIDE;

private:
    void keyEvent(uint code, uint32_t state);
    void sendKeyEvent(uint code, uint32_t state);
    void updateModifierState(uint code, uint32_t state);
    void updateKeymap();

#ifndef QT_NO_WAYLAND_XKB
    void initXKB();
    void createXKBKeymap();
#endif

    QWaylandInputDevice *m_seat;

    QWaylandKeyboardGrabber* m_grab;
    QWaylandSurface *m_focus;
    Resource *m_focusResource;
    QWaylandDestroyListener m_focusDestroyListener;

    QVector<uint32_t> m_keys;
    uint32_t m_modsDepressed;
    uint32_t m_modsLatched;
    uint32_t m_modsLocked;
    uint32_t m_group;

    QWaylandKeymap m_keymap;
    bool m_pendingKeymap;
#ifndef QT_NO_WAYLAND_XKB
    size_t m_keymap_size;
    int m_keymap_fd;
    char *m_keymap_area;
    struct xkb_context *m_context;
    struct xkb_state *m_state;
#endif
};

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLKEYBOARD_P_H
