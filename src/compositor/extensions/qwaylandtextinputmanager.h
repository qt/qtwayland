// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTMANAGER_H
#define QWAYLANDTEXTINPUTMANAGER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandTextInputManagerPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputManager : public QWaylandCompositorExtensionTemplate<QWaylandTextInputManager>
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
