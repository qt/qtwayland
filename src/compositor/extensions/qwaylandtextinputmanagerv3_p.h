// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTEXTINPUTMANAGERV3_P_H
#define QWAYLANDTEXTINPUTMANAGERV3_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>

#include <QtWaylandCompositor/private/qwayland-server-text-input-unstable-v3.h>

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextInputManagerV3Private : public QWaylandCompositorExtensionPrivate, public QtWaylandServer::zwp_text_input_manager_v3
{
    Q_DECLARE_PUBLIC(QWaylandTextInputManagerV3)
public:
    QWaylandTextInputManagerV3Private();

protected:
    void zwp_text_input_manager_v3_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTMANAGERV3_P_H
