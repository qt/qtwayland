// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "textinput.h"

namespace MockCompositor {

TextInputManager::TextInputManager(CoreCompositor *compositor)
{
    init(compositor->m_display, 1);
}

void TextInputManager::zwp_text_input_manager_v2_get_text_input(Resource *resource, uint32_t id, wl_resource *seatResource)
{
    Q_UNUSED(seatResource);
    add(resource->client(), id, resource->version());
}

} // namespace MockCompositor
