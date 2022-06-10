// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTWAYLAND_QWLKEYBOARD_P_H
#define QTWAYLAND_QWLKEYBOARD_P_H

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
#include <QtWaylandCompositor/private/qwaylandcompositor_p.h>

#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>
#include <QtWaylandCompositor/qwaylandseat.h>
#include <QtWaylandCompositor/qwaylandkeyboard.h>
#include <QtWaylandCompositor/qwaylanddestroylistener.h>

#include <QtCore/private/qobject_p.h>
#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

#include <QtCore/QList>

#if QT_CONFIG(xkbcommon)
#include <xkbcommon/xkbcommon.h>
#include <QtGui/private/qxkbcommon_p.h>
#endif


QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandKeyboardPrivate : public QObjectPrivate
                                                  , public QtWaylandServer::wl_keyboard
{
public:
    Q_DECLARE_PUBLIC(QWaylandKeyboard)

    static QWaylandKeyboardPrivate *get(QWaylandKeyboard *keyboard);

    QWaylandKeyboardPrivate(QWaylandSeat *seat);
    ~QWaylandKeyboardPrivate() override;

    QWaylandCompositor *compositor() const { return seat->compositor(); }

    void focused(QWaylandSurface* surface);

#if QT_CONFIG(xkbcommon)
    struct xkb_state *xkbState() const { return mXkbState.get(); }
    struct xkb_context *xkbContext() const {
        return QWaylandCompositorPrivate::get(seat->compositor())->xkbContext();
    }
    uint32_t xkbModsMask() const { return modsDepressed | modsLatched | modsLocked; }
    void maybeUpdateXkbScanCodeTable();
    void resetKeyboardState();
#endif

    void keyEvent(uint code, uint32_t state);
    void sendKeyEvent(uint code, uint32_t state);
    void updateModifierState(uint code, uint32_t state);
    void checkAndRepairModifierState(QKeyEvent *ke);
    void maybeUpdateKeymap();

    void checkFocusResource(Resource *resource);
    void sendEnter(QWaylandSurface *surface, Resource *resource);

protected:
    void keyboard_bind_resource(Resource *resource) override;
    void keyboard_destroy_resource(Resource *resource) override;
    void keyboard_release(Resource *resource) override;

private:
#if QT_CONFIG(xkbcommon)
    void createXKBKeymap();
    void createXKBState(xkb_keymap *keymap);
#endif
    static uint toWaylandKey(const uint nativeScanCode);
    static uint fromWaylandKey(const uint key);

    void sendRepeatInfo();

    QWaylandSeat *seat = nullptr;

    QWaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    QWaylandDestroyListener focusDestroyListener;

    QList<uint32_t> keys;
    uint32_t modsDepressed = 0;
    uint32_t modsLatched = 0;
    uint32_t modsLocked = 0;
    uint32_t group = 0;

    uint32_t shiftIndex = 0;
    uint32_t controlIndex = 0;
    uint32_t altIndex = 0;

    Qt::KeyboardModifiers currentModifierState;

    bool pendingKeymap = false;
#if QT_CONFIG(xkbcommon)
    size_t keymap_size;
    int keymap_fd = -1;
    char *keymap_area = nullptr;
    using ScanCodeKey = std::pair<uint,int>; // group/layout and QtKey
    QMap<ScanCodeKey, uint> scanCodesByQtKey;
    QXkbCommon::ScopedXKBState mXkbState;
#endif

    quint32 repeatRate = 40;
    quint32 repeatDelay = 400;
};

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLKEYBOARD_P_H
