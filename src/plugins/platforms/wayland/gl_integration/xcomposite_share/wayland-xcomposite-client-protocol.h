#ifndef XCOMPOSITE_CLIENT_PROTOCOL_H
#define XCOMPOSITE_CLIENT_PROTOCOL_H

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

struct wl_xcomposite_listener {
	void (*root)(void *data,
		     struct wl_xcomposite *wl_xcomposite,
		     const char *display_name,
		     uint32_t root_window);
};

static inline int
wl_xcomposite_add_listener(struct wl_xcomposite *wl_xcomposite,
			   const struct wl_xcomposite_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) wl_xcomposite,
				     (void (**)(void)) listener, data);
}

#define WL_XCOMPOSITE_CREATE_BUFFER	0

static inline void
wl_xcomposite_set_user_data(struct wl_xcomposite *wl_xcomposite, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) wl_xcomposite, user_data);
}

static inline void *
wl_xcomposite_get_user_data(struct wl_xcomposite *wl_xcomposite)
{
	return wl_proxy_get_user_data((struct wl_proxy *) wl_xcomposite);
}

static inline void
wl_xcomposite_destroy(struct wl_xcomposite *wl_xcomposite)
{
	wl_proxy_destroy((struct wl_proxy *) wl_xcomposite);
}

static inline struct wl_buffer *
wl_xcomposite_create_buffer(struct wl_xcomposite *wl_xcomposite, uint32_t x_window, int32_t width, int32_t height)
{
	struct wl_proxy *id;

	id = wl_proxy_create((struct wl_proxy *) wl_xcomposite,
			     &wl_buffer_interface);
	if (!id)
		return NULL;

	wl_proxy_marshal((struct wl_proxy *) wl_xcomposite,
			 WL_XCOMPOSITE_CREATE_BUFFER, id, x_window, width, height);

	return (struct wl_buffer *) id;
}

#ifdef  __cplusplus
}
#endif

#endif
