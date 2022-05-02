/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQtTextInputMethodManagerPrivate : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::qt_text_input_method_manager_v1
{
    Q_DECLARE_PUBLIC(QWaylandQtTextInputMethodManager)
public:
    QWaylandQtTextInputMethodManagerPrivate();

protected:
    void text_input_method_manager_v1_get_text_input_method(Resource *resource, uint32_t id, struct ::wl_resource *seat) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDQTTEXTINPUTMETHODMANAGER_P_H
