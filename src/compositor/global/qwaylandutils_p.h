// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDUTILS_P_H
#define QWAYLANDUTILS_P_H

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

#include <QtCore/private/qglobal_p.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

namespace QtWayland {

template<typename return_type>
return_type fromResource(struct ::wl_resource *resource) {
    if (auto *r = std::remove_pointer<return_type>::type::Resource::fromResource(resource))
        return static_cast<return_type>(r->object());
    return nullptr;
}

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QWAYLANDUTILS_P_H
