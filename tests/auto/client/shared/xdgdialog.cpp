// Copyright (C) 2024 David Redondo <kde@david-redondo.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "xdgdialog.h"

#include "xdgshell.h"

namespace MockCompositor {

XdgDialog::XdgDialog(XdgWmDialog *wm, XdgToplevel *toplevel, wl_client *client, int id, int version)
    : QtWaylandServer::xdg_dialog_v1(client, id, version),
      toplevel(toplevel),
      modal(false),
      m_wm(wm)
{
}

void XdgDialog::xdg_dialog_v1_set_modal(Resource *resource)
{
    Q_UNUSED(resource)
    modal = true;
}

void XdgDialog::xdg_dialog_v1_unset_modal(Resource *resource)
{
    Q_UNUSED(resource)
    modal = false;
}

void XdgDialog::xdg_dialog_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgDialog::xdg_dialog_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    m_wm->m_dialogs.removeOne(this);
    delete this;
}

XdgWmDialog::XdgWmDialog(CoreCompositor *compositor, int version)
    : QtWaylandServer::xdg_wm_dialog_v1(compositor->m_display, version)
{
}

void XdgWmDialog::xdg_wm_dialog_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgWmDialog::xdg_wm_dialog_v1_get_xdg_dialog(Resource *resource, uint32_t id,
                                                  struct ::wl_resource *toplevel)
{
    auto *t = fromResource<XdgToplevel>(toplevel);
    auto *dialog = new XdgDialog(this, t, resource->client(), id, resource->version());
    m_dialogs.push_back(dialog);
}

} // namespace MockCompositor
