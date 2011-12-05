/* 
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (qt-info@nokia.com)
 * 
 * This file is part of the plugins of the Qt Toolkit.
 * 
 * $QT_BEGIN_LICENSE:LGPL$
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation and
 * appearing in the file LICENSE.LGPL included in the packaging of this
 * file. Please review the following information to ensure the GNU Lesser
 * General Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 * 
 * In addition, as a special exception, Nokia gives you certain additional
 * rights. These rights are described in the Nokia Qt LGPL Exception
 * version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 * 
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 3.0 as published by the Free Software Foundation
 * and appearing in the file LICENSE.GPL included in the packaging of this
 * file. Please review the following information to ensure the GNU General
 * Public License version 3.0 requirements will be met:
 * http://www.gnu.org/copyleft/gpl.html.
 * 
 * Other Usage
 * Alternatively, this file may be used in accordance with the terms and
 * conditions contained in a signed written agreement between you and Nokia.
 * 
 * 
 * 
 * 
 * 
 * $QT_END_LICENSE$
 */

#ifndef WAYLAND_WINDOWMANAGER_SERVER_PROTOCOL_H
#define WAYLAND_WINDOWMANAGER_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;
struct wl_resource;

struct wl_windowmanager;

extern const struct wl_interface wl_windowmanager_interface;

struct wl_windowmanager_interface {
	void (*map_client_to_process)(struct wl_client *client,
				      struct wl_resource *resource,
				      uint32_t processid);
	void (*authenticate_with_token)(struct wl_client *client,
					struct wl_resource *resource,
					const char *processid);
	void (*update_generic_property)(struct wl_client *client,
					struct wl_resource *resource,
					struct wl_resource *surface,
					const char *name,
					struct wl_array *value);
};

#define WL_WINDOWMANAGER_CLIENT_ONSCREEN_VISIBILITY	0
#define WL_WINDOWMANAGER_SET_SCREEN_ROTATION	1
#define WL_WINDOWMANAGER_SET_GENERIC_PROPERTY	2

#ifdef  __cplusplus
}
#endif

#endif
