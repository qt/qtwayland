// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandcompositor.h"
#include "qwaylandxdgdialogv1_p.h"

#include <QWaylandXdgToplevel>
#include <wayland-server.h>

QT_BEGIN_NAMESPACE

QWaylandXdgDialogV1Global::QWaylandXdgDialogV1Global(QWaylandCompositor *parent)
    : QWaylandCompositorExtensionTemplate<QWaylandXdgDialogV1Global>(parent)
{
}

void QWaylandXdgDialogV1Global::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (compositor)
        init(compositor->display(), 1);
}

void QWaylandXdgDialogV1Global::xdg_wm_dialog_v1_get_xdg_dialog(Resource *resource, uint32_t id, wl_resource *toplevelResource)
{
    auto *toplevel = QWaylandXdgToplevel::fromResource(toplevelResource);
    (void)new QWaylandXdgDialogV1(toplevel, resource->client(), id);
}

QWaylandXdgDialogV1::QWaylandXdgDialogV1(QWaylandXdgToplevel *toplevel, wl_client *client, int id)
    : QtWaylandServer::xdg_dialog_v1(client, id, 1), m_topLevel(toplevel)
{
}

void QWaylandXdgDialogV1::xdg_dialog_v1_set_modal(Resource *resource)
{
    Q_UNUSED(resource);
    if (m_topLevel)
        m_topLevel->setModal(true);

}

void QWaylandXdgDialogV1::xdg_dialog_v1_unset_modal(Resource *resource)
{
    Q_UNUSED(resource);
    if (m_topLevel)
        m_topLevel->setModal(false);
}

void QWaylandXdgDialogV1::xdg_dialog_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void QWaylandXdgDialogV1::xdg_dialog_v1_destroy(Resource *resource)
{
    if (m_topLevel)
        m_topLevel->setModal(false);
    wl_resource_destroy(resource->handle);
}

QT_END_NAMESPACE
