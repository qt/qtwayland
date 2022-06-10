// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTV4_H
#define QWAYLANDTEXTINPUTV4_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

struct wl_client;

QT_BEGIN_NAMESPACE

class QWaylandTextInputV4Private;

class QInputMethodEvent;
class QKeyEvent;
class QWaylandSurface;

class QWaylandTextInputV4 : public QWaylandCompositorExtensionTemplate<QWaylandTextInputV4>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandTextInputV4)
public:
    explicit QWaylandTextInputV4(QWaylandObject *container, QWaylandCompositor *compositor);
    ~QWaylandTextInputV4() override;

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    QWaylandSurface *focus() const;
    void setFocus(QWaylandSurface *surface);

    bool isSurfaceEnabled(QWaylandSurface *surface) const;

    void add(::wl_client *client, uint32_t id, int version);
    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void updateInputMethod(Qt::InputMethodQueries queries);
    void surfaceEnabled(QWaylandSurface *surface);
    void surfaceDisabled(QWaylandSurface *surface);

private:
    void focusSurfaceDestroyed(void *);
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTV4_H
