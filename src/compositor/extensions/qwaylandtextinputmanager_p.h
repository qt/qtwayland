// Copyright (C) 2017-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTMANAGER_P_H
#define QWAYLANDTEXTINPUTMANAGER_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>

#include <QtWaylandCompositor/private/qwayland-server-text-input-unstable-v2.h>

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputManagerPrivate : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::zwp_text_input_manager_v2
{
    Q_DECLARE_PUBLIC(QWaylandTextInputManager)
public:
    QWaylandTextInputManagerPrivate();

protected:
    void zwp_text_input_manager_v2_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTMANAGER_P_H
