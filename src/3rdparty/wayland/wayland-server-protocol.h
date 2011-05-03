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


#ifndef WAYLAND_SERVER_PROTOCOL_H
#define WAYLAND_SERVER_PROTOCOL_H

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

struct wl_display_interface {
	void (*bind)(struct wl_client *client,
		     struct wl_display *wl_display,
		     uint32_t id,
		     const char *interface,
		     uint32_t version);
	void (*sync)(struct wl_client *client,
		     struct wl_display *wl_display,
		     uint32_t key);
	void (*frame)(struct wl_client *client,
		      struct wl_display *wl_display,
		      struct wl_surface *surface,
		      uint32_t key);
};

#define WL_DISPLAY_INVALID_OBJECT	0
#define WL_DISPLAY_INVALID_METHOD	1
#define WL_DISPLAY_NO_MEMORY	2
#define WL_DISPLAY_GLOBAL	3
#define WL_DISPLAY_RANGE	4
#define WL_DISPLAY_KEY	5

struct wl_compositor_interface {
	void (*create_surface)(struct wl_client *client,
			       struct wl_compositor *wl_compositor,
			       uint32_t id);
};

struct wl_shm_interface {
	void (*create_buffer)(struct wl_client *client,
			      struct wl_shm *wl_shm,
			      uint32_t id,
			      int fd,
			      int width,
			      int height,
			      uint32_t stride,
			      struct wl_visual *visual);
};

struct wl_buffer_interface {
	void (*damage)(struct wl_client *client,
		       struct wl_buffer *wl_buffer,
		       int x,
		       int y,
		       int width,
		       int height);
	void (*destroy)(struct wl_client *client,
			struct wl_buffer *wl_buffer);
};

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

struct wl_shell_interface {
	void (*move)(struct wl_client *client,
		     struct wl_shell *wl_shell,
		     struct wl_surface *surface,
		     struct wl_input_device *input_device,
		     uint32_t time);
	void (*resize)(struct wl_client *client,
		       struct wl_shell *wl_shell,
		       struct wl_surface *surface,
		       struct wl_input_device *input_device,
		       uint32_t time,
		       uint32_t edges);
	void (*create_drag)(struct wl_client *client,
			    struct wl_shell *wl_shell,
			    uint32_t id);
	void (*create_selection)(struct wl_client *client,
				 struct wl_shell *wl_shell,
				 uint32_t id);
};

#define WL_SHELL_CONFIGURE	0

struct wl_selection_interface {
	void (*offer)(struct wl_client *client,
		      struct wl_selection *wl_selection,
		      const char *type);
	void (*activate)(struct wl_client *client,
			 struct wl_selection *wl_selection,
			 struct wl_input_device *input_device,
			 uint32_t time);
	void (*destroy)(struct wl_client *client,
			struct wl_selection *wl_selection);
};

#define WL_SELECTION_SEND	0
#define WL_SELECTION_CANCELLED	1

struct wl_selection_offer_interface {
	void (*receive)(struct wl_client *client,
			struct wl_selection_offer *wl_selection_offer,
			const char *mime_type,
			int fd);
};

#define WL_SELECTION_OFFER_OFFER	0
#define WL_SELECTION_OFFER_KEYBOARD_FOCUS	1

struct wl_drag_interface {
	void (*offer)(struct wl_client *client,
		      struct wl_drag *wl_drag,
		      const char *type);
	void (*activate)(struct wl_client *client,
			 struct wl_drag *wl_drag,
			 struct wl_surface *surface,
			 struct wl_input_device *input_device,
			 uint32_t time);
	void (*destroy)(struct wl_client *client,
			struct wl_drag *wl_drag);
};

#define WL_DRAG_TARGET	0
#define WL_DRAG_FINISH	1
#define WL_DRAG_REJECT	2

struct wl_drag_offer_interface {
	void (*accept)(struct wl_client *client,
		       struct wl_drag_offer *wl_drag_offer,
		       uint32_t time,
		       const char *type);
	void (*receive)(struct wl_client *client,
			struct wl_drag_offer *wl_drag_offer,
			int fd);
	void (*reject)(struct wl_client *client,
		       struct wl_drag_offer *wl_drag_offer);
};

#define WL_DRAG_OFFER_OFFER	0
#define WL_DRAG_OFFER_POINTER_FOCUS	1
#define WL_DRAG_OFFER_MOTION	2
#define WL_DRAG_OFFER_DROP	3

struct wl_surface_interface {
	void (*destroy)(struct wl_client *client,
			struct wl_surface *wl_surface);
	void (*attach)(struct wl_client *client,
		       struct wl_surface *wl_surface,
		       struct wl_buffer *buffer,
		       int x,
		       int y);
	void (*map_toplevel)(struct wl_client *client,
			     struct wl_surface *wl_surface);
	void (*map_transient)(struct wl_client *client,
			      struct wl_surface *wl_surface,
			      struct wl_surface *parent,
			      int x,
			      int y,
			      uint32_t flags);
	void (*map_fullscreen)(struct wl_client *client,
			       struct wl_surface *wl_surface);
	void (*damage)(struct wl_client *client,
		       struct wl_surface *wl_surface,
		       int x,
		       int y,
		       int width,
		       int height);
};

struct wl_input_device_interface {
	void (*attach)(struct wl_client *client,
		       struct wl_input_device *wl_input_device,
		       uint32_t time,
		       struct wl_buffer *buffer,
		       int hotspot_x,
		       int hotspot_y);
};

#define WL_INPUT_DEVICE_MOTION	0
#define WL_INPUT_DEVICE_BUTTON	1
#define WL_INPUT_DEVICE_KEY	2
#define WL_INPUT_DEVICE_POINTER_FOCUS	3
#define WL_INPUT_DEVICE_KEYBOARD_FOCUS	4

#define WL_OUTPUT_GEOMETRY	0

#ifdef  __cplusplus
}
#endif

#endif
