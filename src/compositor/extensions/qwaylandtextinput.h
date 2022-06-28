// Copyright (C) 2017-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUT_H
#define QWAYLANDTEXTINPUT_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

struct wl_client;

QT_BEGIN_NAMESPACE

class QWaylandTextInputPrivate;

class QInputMethodEvent;
class QKeyEvent;
class QWaylandSurface;

class QWaylandTextInput : public QWaylandCompositorExtensionTemplate<QWaylandTextInput>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandTextInput)
public:
    explicit QWaylandTextInput(QWaylandObject *container, QWaylandCompositor *compositor);
    ~QWaylandTextInput() override;

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    QWaylandSurface *focus() const;
    void setFocus(QWaylandSurface *surface);

    bool isSurfaceEnabled(QWaylandSurface *surface) const;

    void add(::wl_client *client, uint32_t id, int version);
    static const struct wl_interface *interface();
    static QByteArray interfaceName();

    void sendModifiersMap(const QByteArray &modifiersMap);

Q_SIGNALS:
    void updateInputMethod(Qt::InputMethodQueries queries);
    void surfaceEnabled(QWaylandSurface *surface);
    void surfaceDisabled(QWaylandSurface *surface);

private:
    void focusSurfaceDestroyed(void *);
    void sendInputPanelState();
    void sendTextDirection();
    void sendLocale();
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUT_H
