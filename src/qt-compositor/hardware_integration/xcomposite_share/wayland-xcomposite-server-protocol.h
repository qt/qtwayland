#ifndef XCOMPOSITE_SERVER_PROTOCOL_H
#define XCOMPOSITE_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;
struct wl_resource;

struct wl_xcomposite;

extern const struct wl_interface wl_xcomposite_interface;

struct wl_xcomposite_interface {
	void (*create_buffer)(struct wl_client *client,
			      struct wl_resource *resource,
			      uint32_t id,
			      uint32_t x_window,
			      int32_t width,
			      int32_t height);
};

#define WL_XCOMPOSITE_ROOT	0

#ifdef  __cplusplus
}
#endif

#endif
