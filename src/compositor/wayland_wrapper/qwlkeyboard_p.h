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

#include <QObject>
#include <QtCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QVector>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#endif

#include "qwllistener_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Compositor;
class InputDevice;
class Surface;
class Keyboard;

class Q_COMPOSITOR_EXPORT KeyboardGrabber {
    public:
        virtual ~KeyboardGrabber();
        virtual void focused(Surface *surface) = 0;
        virtual void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) = 0;
        virtual void modifiers(uint32_t serial, uint32_t mods_depressed,
                uint32_t mods_latched, uint32_t mods_locked, uint32_t group) = 0;

        Keyboard *m_keyboard;
};

class Q_COMPOSITOR_EXPORT Keyboard : public QObject, public QtWaylandServer::wl_keyboard, public KeyboardGrabber
{
    Q_OBJECT

public:
    Keyboard(Compositor *compositor, InputDevice *seat);
    ~Keyboard();

    void setFocus(Surface *surface);
    void setKeymap(const QWaylandKeymap &keymap);

    void sendKeyModifiers(Resource *resource, uint32_t serial);
    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    Surface *focus() const;
    Resource *focusResource() const;

    void focused(Surface* surface);
    void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void modifiers(uint32_t serial, uint32_t mods_depressed,
                uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

    void keyEvent(uint code, uint32_t state);
    void updateModifierState(uint code, uint32_t state);
    void updateKeymap();

   void startGrab(KeyboardGrabber *grab);
   void endGrab();
   KeyboardGrabber *currentGrab() const;

#ifndef QT_NO_WAYLAND_XKB
    struct xkb_state *xkbState() const { return m_state; }
    uint32_t xkbModsMask() const { return m_modsDepressed | m_modsLatched | m_modsLocked; }
#endif

Q_SIGNALS:
    void focusChanged(Surface *surface);

protected:
    void keyboard_bind_resource(Resource *resource);
    void keyboard_destroy_resource(Resource *resource);
    void keyboard_release(Resource *resource) Q_DECL_OVERRIDE;

private:
    void sendKeyEvent(uint code, uint32_t state);
    void focusDestroyed(void *data);

#ifndef QT_NO_WAYLAND_XKB
    void initXKB();
    void createXKBKeymap();
#endif

    Compositor *m_compositor;
    InputDevice *m_seat;

    KeyboardGrabber* m_grab;
    Surface *m_focus;
    Resource *m_focusResource;
    WlListener m_focusDestroyListener;

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

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLKEYBOARD_P_H
