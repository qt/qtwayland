// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQTTEXTINPUTMETHODMANAGER_P_H
#define QWAYLANDQTTEXTINPUTMETHODMANAGER_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>

#include <QtWaylandCompositor/private/qwayland-server-qt-text-input-method-unstable-v1.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtTextInputMethodManagerPrivate : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::qt_text_input_method_manager_v1
{
    Q_DECLARE_PUBLIC(QWaylandQtTextInputMethodManager)
public:
    QWaylandQtTextInputMethodManagerPrivate();

protected:
    void text_input_method_manager_v1_get_text_input_method(Resource *resource, uint32_t id, struct ::wl_resource *seat) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDQTTEXTINPUTMETHODMANAGER_P_H
