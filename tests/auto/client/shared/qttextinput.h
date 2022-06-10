// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MOCKCOMPOSITOR_QTTEXTINPUT_H
#define MOCKCOMPOSITOR_QTTEXTINPUT_H

#include "coreprotocol.h"
#include <qwayland-server-qt-text-input-method-unstable-v1.h>

#include <QtGui/qpa/qplatformnativeinterface.h>

namespace MockCompositor {

class QtTextInputManager : public Global, public QtWaylandServer::qt_text_input_method_manager_v1
{
    Q_OBJECT
public:
    QtTextInputManager(CoreCompositor *compositor);

protected:
    void text_input_method_manager_v1_get_text_input_method(Resource *resource, uint32_t id, struct ::wl_resource *seatResource) override;
};

} // namespace MockCompositor

#endif // MOCKCOMPOSITOR_QTTEXTINPUT_H
