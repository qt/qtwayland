// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQTTEXTINPUTMETHODMANAGER_H
#define QWAYLANDQTTEXTINPUTMETHODMANAGER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandQtTextInputMethodManagerPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtTextInputMethodManager : public QWaylandCompositorExtensionTemplate<QWaylandQtTextInputMethodManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQtTextInputMethodManager)
public:
    QWaylandQtTextInputMethodManager();
    QWaylandQtTextInputMethodManager(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
};

QT_END_NAMESPACE

#endif // QWAYLANDQTTEXTINPUTMETHODMANAGER_H
