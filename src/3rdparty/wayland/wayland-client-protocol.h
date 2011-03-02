/*
 * Copyright © 2010 Kristian Høgsberg
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */


#ifndef WAYLAND_CLIENT_PROTOCOL_H
#define WAYLAND_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;

struct wl_display;
struct wl_compositor;
struct wl_shm;
struct wl_buffer;
struct wl_shell;
struct wl_selection;
struct wl_selection_offer;
struct wl_drag;
struct wl_drag_offer;
struct wl_surface;
struct wl_input_device;
struct wl_output;
struct wl_visual;

struct wl_proxy;

extern void
wl_proxy_marshal(struct wl_proxy *p, uint32_t opcode, ...);
extern struct wl_proxy *
wl_proxy_create(struct wl_proxy *factory,
		const struct wl_interface *interface);
extern struct wl_proxy *
wl_proxy_create_for_id(struct wl_display *display,
		       const struct wl_interface *interface, uint32_t id);
extern void
wl_proxy_destroy(struct wl_proxy *proxy);

extern int
wl_proxy_add_listener(struct wl_proxy *proxy,
		      void (**implementation)(void), void *data);

extern void
wl_proxy_set_user_data(struct wl_proxy *proxy, void *user_data);

extern void *
wl_proxy_get_user_data(struct wl_proxy *proxy);

extern const struct wl_interface wl_display_interface;
extern const struct wl_interface wl_compositor_interface;
extern const struct wl_interface wl_shm_interface;
extern const struct wl_interface wl_buffer_interface;
extern const struct wl_interface wl_shell_interface;
extern const struct wl_interface wl_selection_interface;
extern const struct wl_interface wl_selection_offer_interface;
extern const struct wl_interface wl_drag_interface;
extern const struct wl_interface wl_drag_offer_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_input_device_interface;
extern const struct wl_interface wl_output_interface;
extern const struct wl_interface wl_visual_interface;

struct wl_display_listener {
	void (*invalid_object)(void *data,
			       struct wl_display *display,
			       uint32_t object_id);
	void (*invalid_method)(void *data,
			       struct wl_display *display,
			       uint32_t object_id,
			       uint32_t opcode);
	void (*no_memory)(void *data,
			  struct wl_display *display);
	void (*global)(void *data,
		       struct wl_display *display,
		       uint32_t id,
		       const char *name,
		       uint32_t version);
	void (*range)(void *data,
		      struct wl_display *display,
		      uint32_t base);
	void (*key)(void *data,
		    struct wl_display *display,
		    uint32_t key,
		    uint32_t time);
};

static inline int
wl_display_add_listener(struct wl_display *display,
			const struct wl_display_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) display,
				     (void (**)(void)) listener, data);
}

#define WL_DISPLAY_SYNC	0
#define WL_DISPLAY_FRAME	1

static inline void
wl_display_set_user_data(struct wl_display *display, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) display, user_data);
}

static inline void *
wl_display_get_user_data(struct wl_display *display)
{
	return wl_proxy_get_user_data((struct wl_proxy *) display);
}

static inline void
wl_display_sync(struct wl_display *display, uint32_t key)
{
	wl_proxy_marshal((struct wl_proxy *) display,
			 WL_DISPLAY_SYNC, key);
}

static inline void
wl_display_frame(struct wl_display *display, uint32_t key)
{
	wl_proxy_marshal((struct wl_proxy *) display,
			 WL_DISPLAY_FRAME, key);
}

#define WL_COMPOSITOR_CREATE_SURFACE	0

static inline struct wl_compositor *
wl_compositor_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_compositor *)
		wl_proxy_create_for_id(display, &wl_compositor_interface, id);
}

static inline void
wl_compositor_set_user_data(struct wl_compositor *compositor, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) compositor, user_data);
}

static inline void *
wl_compositor_get_user_data(struct wl_compositor *compositor)
{
	return wl_proxy_get_user_data((struct wl_proxy *) compositor);
}

static inline void
wl_compositor_destroy(struct wl_compositor *compositor)
{
	wl_proxy_destroy((struct wl_proxy *) compositor);
}

static inline struct wl_surface *
wl_compositor_create_surface(struct wl_compositor *compositor)
{
	struct wl_proxy *id;

	id = wl_proxy_create((struct wl_proxy *) compositor,
			     &wl_surface_interface);
	if (!id)
		return NULL;

	wl_proxy_marshal((struct wl_proxy *) compositor,
			 WL_COMPOSITOR_CREATE_SURFACE, id);

	return (struct wl_surface *) id;
}

#define WL_SHM_CREATE_BUFFER	0

static inline struct wl_shm *
wl_shm_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_shm *)
		wl_proxy_create_for_id(display, &wl_shm_interface, id);
}

static inline void
wl_shm_set_user_data(struct wl_shm *shm, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) shm, user_data);
}

static inline void *
wl_shm_get_user_data(struct wl_shm *shm)
{
	return wl_proxy_get_user_data((struct wl_proxy *) shm);
}

static inline void
wl_shm_destroy(struct wl_shm *shm)
{
	wl_proxy_destroy((struct wl_proxy *) shm);
}

static inline struct wl_buffer *
wl_shm_create_buffer(struct wl_shm *shm, int fd, int width, int height, uint32_t stride, struct wl_visual *visual)
{
	struct wl_proxy *id;

	id = wl_proxy_create((struct wl_proxy *) shm,
			     &wl_buffer_interface);
	if (!id)
		return NULL;

	wl_proxy_marshal((struct wl_proxy *) shm,
			 WL_SHM_CREATE_BUFFER, id, fd, width, height, stride, visual);

	return (struct wl_buffer *) id;
}

#define WL_BUFFER_DESTROY	0

static inline struct wl_buffer *
wl_buffer_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_buffer *)
		wl_proxy_create_for_id(display, &wl_buffer_interface, id);
}

static inline void
wl_buffer_set_user_data(struct wl_buffer *buffer, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) buffer, user_data);
}

static inline void *
wl_buffer_get_user_data(struct wl_buffer *buffer)
{
	return wl_proxy_get_user_data((struct wl_proxy *) buffer);
}

static inline void
wl_buffer_destroy(struct wl_buffer *buffer)
{
	wl_proxy_marshal((struct wl_proxy *) buffer,
			 WL_BUFFER_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) buffer);
}

#ifndef WL_SHELL_RESIZE_ENUM
#define WL_SHELL_RESIZE_ENUM
enum wl_shell_resize {
	WL_SHELL_RESIZE_NONE = 0,
	WL_SHELL_RESIZE_TOP = 1,
	WL_SHELL_RESIZE_BOTTOM = 2,
	WL_SHELL_RESIZE_LEFT = 4,
	WL_SHELL_RESIZE_TOP_LEFT = 5,
	WL_SHELL_RESIZE_BOTTOM_LEFT = 6,
	WL_SHELL_RESIZE_RIGHT = 8,
	WL_SHELL_RESIZE_TOP_RIGHT = 9,
	WL_SHELL_RESIZE_BOTTOM_RIGHT = 10,
};
#endif /* WL_SHELL_RESIZE_ENUM */

struct wl_shell_listener {
	void (*configure)(void *data,
			  struct wl_shell *shell,
			  uint32_t time,
			  uint32_t edges,
			  struct wl_surface *surface,
			  int width,
			  int height);
};

static inline int
wl_shell_add_listener(struct wl_shell *shell,
		      const struct wl_shell_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) shell,
				     (void (**)(void)) listener, data);
}

#define WL_SHELL_MOVE	0
#define WL_SHELL_RESIZE	1
#define WL_SHELL_CREATE_DRAG	2
#define WL_SHELL_CREATE_SELECTION	3

static inline struct wl_shell *
wl_shell_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_shell *)
		wl_proxy_create_for_id(display, &wl_shell_interface, id);
}

static inline void
wl_shell_set_user_data(struct wl_shell *shell, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) shell, user_data);
}

static inline void *
wl_shell_get_user_data(struct wl_shell *shell)
{
	return wl_proxy_get_user_data((struct wl_proxy *) shell);
}

static inline void
wl_shell_destroy(struct wl_shell *shell)
{
	wl_proxy_destroy((struct wl_proxy *) shell);
}

static inline void
wl_shell_move(struct wl_shell *shell, struct wl_surface *surface, struct wl_input_device *input_device, uint32_t time)
{
	wl_proxy_marshal((struct wl_proxy *) shell,
			 WL_SHELL_MOVE, surface, input_device, time);
}

static inline void
wl_shell_resize(struct wl_shell *shell, struct wl_surface *surface, struct wl_input_device *input_device, uint32_t time, uint32_t edges)
{
	wl_proxy_marshal((struct wl_proxy *) shell,
			 WL_SHELL_RESIZE, surface, input_device, time, edges);
}

static inline struct wl_drag *
wl_shell_create_drag(struct wl_shell *shell)
{
	struct wl_proxy *id;

	id = wl_proxy_create((struct wl_proxy *) shell,
			     &wl_drag_interface);
	if (!id)
		return NULL;

	wl_proxy_marshal((struct wl_proxy *) shell,
			 WL_SHELL_CREATE_DRAG, id);

	return (struct wl_drag *) id;
}

static inline struct wl_selection *
wl_shell_create_selection(struct wl_shell *shell)
{
	struct wl_proxy *id;

	id = wl_proxy_create((struct wl_proxy *) shell,
			     &wl_selection_interface);
	if (!id)
		return NULL;

	wl_proxy_marshal((struct wl_proxy *) shell,
			 WL_SHELL_CREATE_SELECTION, id);

	return (struct wl_selection *) id;
}

struct wl_selection_listener {
	void (*send)(void *data,
		     struct wl_selection *selection,
		     const char *mime_type,
		     int fd);
	void (*cancelled)(void *data,
			  struct wl_selection *selection);
};

static inline int
wl_selection_add_listener(struct wl_selection *selection,
			  const struct wl_selection_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) selection,
				     (void (**)(void)) listener, data);
}

#define WL_SELECTION_OFFER	0
#define WL_SELECTION_ACTIVATE	1
#define WL_SELECTION_DESTROY	2

static inline struct wl_selection *
wl_selection_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_selection *)
		wl_proxy_create_for_id(display, &wl_selection_interface, id);
}

static inline void
wl_selection_set_user_data(struct wl_selection *selection, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) selection, user_data);
}

static inline void *
wl_selection_get_user_data(struct wl_selection *selection)
{
	return wl_proxy_get_user_data((struct wl_proxy *) selection);
}

static inline void
wl_selection_offer(struct wl_selection *selection, const char *type)
{
	wl_proxy_marshal((struct wl_proxy *) selection,
			 WL_SELECTION_OFFER, type);
}

static inline void
wl_selection_activate(struct wl_selection *selection, struct wl_input_device *input_device, uint32_t time)
{
	wl_proxy_marshal((struct wl_proxy *) selection,
			 WL_SELECTION_ACTIVATE, input_device, time);
}

static inline void
wl_selection_destroy(struct wl_selection *selection)
{
	wl_proxy_marshal((struct wl_proxy *) selection,
			 WL_SELECTION_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) selection);
}

struct wl_selection_offer_listener {
	void (*offer)(void *data,
		      struct wl_selection_offer *selection_offer,
		      const char *type);
	void (*keyboard_focus)(void *data,
			       struct wl_selection_offer *selection_offer,
			       struct wl_input_device *input_device);
};

static inline int
wl_selection_offer_add_listener(struct wl_selection_offer *selection_offer,
				const struct wl_selection_offer_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) selection_offer,
				     (void (**)(void)) listener, data);
}

#define WL_SELECTION_OFFER_RECEIVE	0

static inline struct wl_selection_offer *
wl_selection_offer_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_selection_offer *)
		wl_proxy_create_for_id(display, &wl_selection_offer_interface, id);
}

static inline void
wl_selection_offer_set_user_data(struct wl_selection_offer *selection_offer, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) selection_offer, user_data);
}

static inline void *
wl_selection_offer_get_user_data(struct wl_selection_offer *selection_offer)
{
	return wl_proxy_get_user_data((struct wl_proxy *) selection_offer);
}

static inline void
wl_selection_offer_destroy(struct wl_selection_offer *selection_offer)
{
	wl_proxy_destroy((struct wl_proxy *) selection_offer);
}

static inline void
wl_selection_offer_receive(struct wl_selection_offer *selection_offer, const char *mime_type, int fd)
{
	wl_proxy_marshal((struct wl_proxy *) selection_offer,
			 WL_SELECTION_OFFER_RECEIVE, mime_type, fd);
}

struct wl_drag_listener {
	void (*target)(void *data,
		       struct wl_drag *drag,
		       const char *mime_type);
	void (*finish)(void *data,
		       struct wl_drag *drag,
		       int fd);
	void (*reject)(void *data,
		       struct wl_drag *drag);
};

static inline int
wl_drag_add_listener(struct wl_drag *drag,
		     const struct wl_drag_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) drag,
				     (void (**)(void)) listener, data);
}

#define WL_DRAG_OFFER	0
#define WL_DRAG_ACTIVATE	1
#define WL_DRAG_DESTROY	2

static inline struct wl_drag *
wl_drag_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_drag *)
		wl_proxy_create_for_id(display, &wl_drag_interface, id);
}

static inline void
wl_drag_set_user_data(struct wl_drag *drag, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) drag, user_data);
}

static inline void *
wl_drag_get_user_data(struct wl_drag *drag)
{
	return wl_proxy_get_user_data((struct wl_proxy *) drag);
}

static inline void
wl_drag_offer(struct wl_drag *drag, const char *type)
{
	wl_proxy_marshal((struct wl_proxy *) drag,
			 WL_DRAG_OFFER, type);
}

static inline void
wl_drag_activate(struct wl_drag *drag, struct wl_surface *surface, struct wl_input_device *input_device, uint32_t time)
{
	wl_proxy_marshal((struct wl_proxy *) drag,
			 WL_DRAG_ACTIVATE, surface, input_device, time);
}

static inline void
wl_drag_destroy(struct wl_drag *drag)
{
	wl_proxy_marshal((struct wl_proxy *) drag,
			 WL_DRAG_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) drag);
}

struct wl_drag_offer_listener {
	void (*offer)(void *data,
		      struct wl_drag_offer *drag_offer,
		      const char *type);
	void (*pointer_focus)(void *data,
			      struct wl_drag_offer *drag_offer,
			      uint32_t time,
			      struct wl_surface *surface,
			      int x,
			      int y,
			      int surface_x,
			      int surface_y);
	void (*motion)(void *data,
		       struct wl_drag_offer *drag_offer,
		       uint32_t time,
		       int x,
		       int y,
		       int surface_x,
		       int surface_y);
	void (*drop)(void *data,
		     struct wl_drag_offer *drag_offer);
};

static inline int
wl_drag_offer_add_listener(struct wl_drag_offer *drag_offer,
			   const struct wl_drag_offer_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) drag_offer,
				     (void (**)(void)) listener, data);
}

#define WL_DRAG_OFFER_ACCEPT	0
#define WL_DRAG_OFFER_RECEIVE	1
#define WL_DRAG_OFFER_REJECT	2

static inline struct wl_drag_offer *
wl_drag_offer_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_drag_offer *)
		wl_proxy_create_for_id(display, &wl_drag_offer_interface, id);
}

static inline void
wl_drag_offer_set_user_data(struct wl_drag_offer *drag_offer, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) drag_offer, user_data);
}

static inline void *
wl_drag_offer_get_user_data(struct wl_drag_offer *drag_offer)
{
	return wl_proxy_get_user_data((struct wl_proxy *) drag_offer);
}

static inline void
wl_drag_offer_destroy(struct wl_drag_offer *drag_offer)
{
	wl_proxy_destroy((struct wl_proxy *) drag_offer);
}

static inline void
wl_drag_offer_accept(struct wl_drag_offer *drag_offer, uint32_t time, const char *type)
{
	wl_proxy_marshal((struct wl_proxy *) drag_offer,
			 WL_DRAG_OFFER_ACCEPT, time, type);
}

static inline void
wl_drag_offer_receive(struct wl_drag_offer *drag_offer, int fd)
{
	wl_proxy_marshal((struct wl_proxy *) drag_offer,
			 WL_DRAG_OFFER_RECEIVE, fd);
}

static inline void
wl_drag_offer_reject(struct wl_drag_offer *drag_offer)
{
	wl_proxy_marshal((struct wl_proxy *) drag_offer,
			 WL_DRAG_OFFER_REJECT);
}

#define WL_SURFACE_DESTROY	0
#define WL_SURFACE_ATTACH	1
#define WL_SURFACE_MAP_TOPLEVEL	2
#define WL_SURFACE_MAP_TRANSIENT	3
#define WL_SURFACE_MAP_FULLSCREEN	4
#define WL_SURFACE_DAMAGE	5

static inline struct wl_surface *
wl_surface_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_surface *)
		wl_proxy_create_for_id(display, &wl_surface_interface, id);
}

static inline void
wl_surface_set_user_data(struct wl_surface *surface, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) surface, user_data);
}

static inline void *
wl_surface_get_user_data(struct wl_surface *surface)
{
	return wl_proxy_get_user_data((struct wl_proxy *) surface);
}

static inline void
wl_surface_destroy(struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) surface,
			 WL_SURFACE_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) surface);
}

static inline void
wl_surface_attach(struct wl_surface *surface, struct wl_buffer *buffer, int x, int y)
{
	wl_proxy_marshal((struct wl_proxy *) surface,
			 WL_SURFACE_ATTACH, buffer, x, y);
}

static inline void
wl_surface_map_toplevel(struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) surface,
			 WL_SURFACE_MAP_TOPLEVEL);
}

static inline void
wl_surface_map_transient(struct wl_surface *surface, struct wl_surface *parent, int x, int y, uint32_t flags)
{
	wl_proxy_marshal((struct wl_proxy *) surface,
			 WL_SURFACE_MAP_TRANSIENT, parent, x, y, flags);
}

static inline void
wl_surface_map_fullscreen(struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) surface,
			 WL_SURFACE_MAP_FULLSCREEN);
}

static inline void
wl_surface_damage(struct wl_surface *surface, int x, int y, int width, int height)
{
	wl_proxy_marshal((struct wl_proxy *) surface,
			 WL_SURFACE_DAMAGE, x, y, width, height);
}

struct wl_input_device_listener {
	void (*motion)(void *data,
		       struct wl_input_device *input_device,
		       uint32_t time,
		       int x,
		       int y,
		       int surface_x,
		       int surface_y);
	void (*button)(void *data,
		       struct wl_input_device *input_device,
		       uint32_t time,
		       uint32_t button,
		       uint32_t state);
	void (*key)(void *data,
		    struct wl_input_device *input_device,
		    uint32_t time,
		    uint32_t key,
		    uint32_t state);
	void (*pointer_focus)(void *data,
			      struct wl_input_device *input_device,
			      uint32_t time,
			      struct wl_surface *surface,
			      int x,
			      int y,
			      int surface_x,
			      int surface_y);
	void (*keyboard_focus)(void *data,
			       struct wl_input_device *input_device,
			       uint32_t time,
			       struct wl_surface *surface,
			       struct wl_array *keys);
};

static inline int
wl_input_device_add_listener(struct wl_input_device *input_device,
			     const struct wl_input_device_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) input_device,
				     (void (**)(void)) listener, data);
}

#define WL_INPUT_DEVICE_ATTACH	0

static inline struct wl_input_device *
wl_input_device_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_input_device *)
		wl_proxy_create_for_id(display, &wl_input_device_interface, id);
}

static inline void
wl_input_device_set_user_data(struct wl_input_device *input_device, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) input_device, user_data);
}

static inline void *
wl_input_device_get_user_data(struct wl_input_device *input_device)
{
	return wl_proxy_get_user_data((struct wl_proxy *) input_device);
}

static inline void
wl_input_device_destroy(struct wl_input_device *input_device)
{
	wl_proxy_destroy((struct wl_proxy *) input_device);
}

static inline void
wl_input_device_attach(struct wl_input_device *input_device, uint32_t time, struct wl_buffer *buffer, int hotspot_x, int hotspot_y)
{
	wl_proxy_marshal((struct wl_proxy *) input_device,
			 WL_INPUT_DEVICE_ATTACH, time, buffer, hotspot_x, hotspot_y);
}

struct wl_output_listener {
	void (*geometry)(void *data,
			 struct wl_output *output,
			 int x,
			 int y,
			 int width,
			 int height);
};

static inline int
wl_output_add_listener(struct wl_output *output,
		       const struct wl_output_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) output,
				     (void (**)(void)) listener, data);
}

static inline struct wl_output *
wl_output_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_output *)
		wl_proxy_create_for_id(display, &wl_output_interface, id);
}

static inline void
wl_output_set_user_data(struct wl_output *output, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) output, user_data);
}

static inline void *
wl_output_get_user_data(struct wl_output *output)
{
	return wl_proxy_get_user_data((struct wl_proxy *) output);
}

static inline void
wl_output_destroy(struct wl_output *output)
{
	wl_proxy_destroy((struct wl_proxy *) output);
}

static inline struct wl_visual *
wl_visual_create(struct wl_display *display, uint32_t id)
{
	return (struct wl_visual *)
		wl_proxy_create_for_id(display, &wl_visual_interface, id);
}

static inline void
wl_visual_set_user_data(struct wl_visual *visual, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) visual, user_data);
}

static inline void *
wl_visual_get_user_data(struct wl_visual *visual)
{
	return wl_proxy_get_user_data((struct wl_proxy *) visual);
}

static inline void
wl_visual_destroy(struct wl_visual *visual)
{
	wl_proxy_destroy((struct wl_proxy *) visual);
}

#ifdef  __cplusplus
}
#endif

#endif
