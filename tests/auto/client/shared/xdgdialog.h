// Copyright (C) 2024 David Redondo <kde@david-redondo.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MOCKCOMPOSITOR_XDG_DIALOG_H
#define MOCKCOMPOSITOR_XDG_DIALOG_H

#include "corecompositor.h"
#include <qwayland-server-xdg-dialog-v1.h>

namespace MockCompositor {

class XdgToplevel;
class XdgWmDialog;

class XdgDialog : public QtWaylandServer::xdg_dialog_v1
{
public:
    explicit XdgDialog(XdgWmDialog *wm, XdgToplevel *toplevel, wl_client *client, int id,
                       int version);
    XdgToplevel *toplevel;
    bool modal;

protected:
    void xdg_dialog_v1_set_modal(Resource *resource) override;
    void xdg_dialog_v1_unset_modal(Resource *resource) override;
    void xdg_dialog_v1_destroy(Resource *resource) override;
    void xdg_dialog_v1_destroy_resource(Resource *resource) override;

private:
    XdgWmDialog *m_wm;
};

class XdgWmDialog : public Global, public QtWaylandServer::xdg_wm_dialog_v1
{
    Q_OBJECT
public:
    explicit XdgWmDialog(CoreCompositor *compositor, int version = 1);
    ~XdgWmDialog() = default;
    QList<XdgDialog *> m_dialogs;

protected:
    void xdg_wm_dialog_v1_destroy(Resource *resource) override;
    void xdg_wm_dialog_v1_get_xdg_dialog(Resource *resource, uint32_t id,
                                         struct ::wl_resource *toplevel) override;
};

} // namespace MockCompositor

#endif
