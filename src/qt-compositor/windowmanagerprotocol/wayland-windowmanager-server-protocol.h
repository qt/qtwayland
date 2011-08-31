#ifndef WAYLAND_WINDOWMANAGER_SERVER_PROTOCOL_H
#define WAYLAND_WINDOWMANAGER_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;

struct wl_windowmanager;

extern const struct wl_interface wl_windowmanager_interface;

struct wl_windowmanager_interface {
	void (*map_client_to_process)(struct wl_client *client,
				      struct wl_windowmanager *wl_windowmanager,
				      uint32_t processid);
	void (*authenticate_with_token)(struct wl_client *client,
					struct wl_windowmanager *wl_windowmanager,
					const char *processid);
	void (*update_generic_property)(struct wl_client *client,
					struct wl_windowmanager *wl_windowmanager,
					struct wl_surface *surface,
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
