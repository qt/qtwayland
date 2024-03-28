// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTV3_H
#define QWAYLANDTEXTINPUTV3_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

struct wl_client;

QT_BEGIN_NAMESPACE

class QWaylandTextInputV3Private;

class QInputMethodEvent;
class QKeyEvent;
class QWaylandSurface;

class QWaylandTextInputV3 : public QWaylandCompositorExtensionTemplate<QWaylandTextInputV3>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandTextInputV3)
public:
    explicit QWaylandTextInputV3(QWaylandObject *container, QWaylandCompositor *compositor);
    ~QWaylandTextInputV3() override;

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

#endif // QWAYLANDTEXTINPUTV3_H
