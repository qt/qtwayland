/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QWAYLANDKEYBOARD_H
#define QWAYLANDKEYBOARD_H

#include <QtCore/QObject>

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandSurface>

QT_BEGIN_NAMESPACE

class QWaylandKeyboard;
class QWaylandKeyboardPrivate;
class QWaylandSeat;
class QWaylandKeymap;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandKeyboard : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandKeyboard)
    Q_PROPERTY(quint32 repeatRate READ repeatRate WRITE setRepeatRate NOTIFY repeatRateChanged)
    Q_PROPERTY(quint32 repeatDelay READ repeatDelay WRITE setRepeatDelay NOTIFY repeatDelayChanged)
public:
    QWaylandKeyboard(QWaylandSeat *seat, QObject *parent = nullptr);

    QWaylandSeat *seat() const;
    QWaylandCompositor *compositor() const;

    quint32 repeatRate() const;
    void setRepeatRate(quint32 rate);

    quint32 repeatDelay() const;
    void setRepeatDelay(quint32 delay);

    virtual void setFocus(QWaylandSurface *surface);

    virtual void sendKeyModifiers(QWaylandClient *client, uint32_t serial);
    virtual void sendKeyPressEvent(uint code);
    virtual void sendKeyReleaseEvent(uint code);

    QWaylandSurface *focus() const;
    QWaylandClient *focusClient() const;

    virtual void addClient(QWaylandClient *client, uint32_t id, uint32_t version);

    uint keyToScanCode(int qtKey) const;

Q_SIGNALS:
    void focusChanged(QWaylandSurface *surface);
    void repeatRateChanged(quint32 repeatRate);
    void repeatDelayChanged(quint32 repeatDelay);

private:
    void focusDestroyed(void *data);

private Q_SLOTS:
    void updateKeymap();
};

QT_END_NAMESPACE

#endif //QWAYLANDKEYBOARD_H
