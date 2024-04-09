// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDXDGDIALOGV1_P_H
#define QWAYLANDXDGDIALOGV1_P_H
#include "qwaylandxdgshell.h"
#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-xdg-dialog-v1.h>

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgDialogV1Global
    : public QWaylandCompositorExtensionTemplate<QWaylandXdgDialogV1Global>
    , public QtWaylandServer::xdg_wm_dialog_v1
{
    Q_OBJECT
public:
    QWaylandXdgDialogV1Global(QWaylandCompositor *parent = nullptr);

    void initialize() override;

protected:
    void xdg_wm_dialog_v1_get_xdg_dialog(Resource *resource, uint32_t id, wl_resource *toplevelResource) override;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgDialogV1
    : public QtWaylandServer::xdg_dialog_v1
{
public:
    QWaylandXdgDialogV1(QWaylandXdgToplevel *toplevel, wl_client *client, int id);

protected:
    void xdg_dialog_v1_destroy_resource(Resource *resource) override;
    void xdg_dialog_v1_destroy(Resource *resource) override;

    void xdg_dialog_v1_set_modal(Resource *resource) override;
    void xdg_dialog_v1_unset_modal(Resource *resource) override;

private:
    QPointer<QWaylandXdgToplevel> m_topLevel;
};

QT_END_NAMESPACE

#endif
