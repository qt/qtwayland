/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidi ary(-ies).
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

#ifndef QWAYLANDKEYBOARD_H
#define QWAYLANDKEYBOARD_H

#include <QtCore/QObject>

#include <QtCompositor/QWaylandExtensionContainer>
#include <QtCompositor/QWaylandSurface>

QT_BEGIN_NAMESPACE

class QWaylandKeyboard;
class QWaylandKeyboardPrivate;
class QWaylandInputDevice;

class Q_COMPOSITOR_EXPORT QWaylandKeyboardGrabber {
public:
    virtual ~QWaylandKeyboardGrabber();
    virtual void focused(QWaylandSurface *surface) = 0;
    virtual void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) = 0;
    virtual void modifiers(uint32_t serial, uint32_t mods_depressed,
                           uint32_t mods_latched, uint32_t mods_locked, uint32_t group) = 0;

    QWaylandKeyboard *m_keyboard;
};

class Q_COMPOSITOR_EXPORT QWaylandKeymap
{
public:
    QWaylandKeymap(const QString &layout = QLatin1String("us"), const QString &variant = QString(), const QString &options = QString(),
                   const QString &model = QLatin1String("pc105"), const QString &rules = QLatin1String("evdev"));

    inline QString layout() const { return m_layout; }
    inline QString variant() const { return m_variant; }
    inline QString options() const { return m_options; }
    inline QString rules() const { return m_rules; }
    inline QString model() const { return m_model; }

private:
    QString m_layout;
    QString m_variant;
    QString m_options;
    QString m_rules;
    QString m_model;
};

class Q_COMPOSITOR_EXPORT QWaylandKeyboard : public QObject, public QWaylandExtensionContainer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandKeyboard)

public:
    QWaylandKeyboard(QWaylandInputDevice *seat, QObject *parent = 0);

    void setFocus(QWaylandSurface *surface);
    virtual void setKeymap(const QWaylandKeymap &keymap);

    virtual void sendKeyModifiers(QWaylandClient *client, uint32_t serial);
    virtual void sendKeyPressEvent(uint code);
    virtual void sendKeyReleaseEvent(uint code);

    QWaylandSurface *focus() const;
    QWaylandClient *focusClient() const;

    void startGrab(QWaylandKeyboardGrabber *grab);
    void endGrab();
    QWaylandKeyboardGrabber *currentGrab() const;

Q_SIGNALS:
    void focusChanged(QWaylandSurface *surface);
private:
    void focusDestroyed(void *data);
};

QT_END_NAMESPACE

#endif //QWAYLANDKEYBOARD_H
