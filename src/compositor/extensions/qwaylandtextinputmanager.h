/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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
******************************************************************************/

#ifndef QWAYLANDTEXTINPUTMANAGER_H
#define QWAYLANDTEXTINPUTMANAGER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandTextInputManagerPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandTextInputManager : public QWaylandCompositorExtensionTemplate<QWaylandTextInputManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandTextInputManager)
public:
    QWaylandTextInputManager();
    QWaylandTextInputManager(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTMANAGER_H
