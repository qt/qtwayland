// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qttextinput.h"

namespace MockCompositor {

QtTextInputManager::QtTextInputManager(CoreCompositor *compositor)
{
    init(compositor->m_display, 1);
}

void QtTextInputManager::text_input_method_manager_v1_get_text_input_method(Resource *resource, uint32_t id, wl_resource *seatResource)
{
    Q_UNUSED(resource);
    Q_UNUSED(id);
    Q_UNUSED(seatResource);
}

} // namespace MockCompositor
