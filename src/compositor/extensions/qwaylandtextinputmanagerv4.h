// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTMANAGERV4_H
#define QWAYLANDTEXTINPUTMANAGERV4_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandTextInputManagerV4Private;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputManagerV4 : public QWaylandCompositorExtensionTemplate<QWaylandTextInputManagerV4>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandTextInputManagerV4)
public:
    QWaylandTextInputManagerV4();
    explicit QWaylandTextInputManagerV4(QWaylandCompositor *compositor);
    ~QWaylandTextInputManagerV4() override;

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTMANAGERV4_H
