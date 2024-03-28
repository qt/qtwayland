// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTMANAGERV3_H
#define QWAYLANDTEXTINPUTMANAGERV3_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandTextInputManagerV3Private;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputManagerV3 : public QWaylandCompositorExtensionTemplate<QWaylandTextInputManagerV3>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandTextInputManagerV3)
public:
    QWaylandTextInputManagerV3();
    explicit QWaylandTextInputManagerV3(QWaylandCompositor *compositor);
    ~QWaylandTextInputManagerV3() override;

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTMANAGERV3_H
