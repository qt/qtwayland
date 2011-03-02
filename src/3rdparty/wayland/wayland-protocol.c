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


#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

static const struct wl_message display_requests[] = {
	{ "sync", "u" },
	{ "frame", "u" },
};

static const struct wl_message display_events[] = {
	{ "invalid_object", "u" },
	{ "invalid_method", "uu" },
	{ "no_memory", "" },
	{ "global", "nsu" },
	{ "range", "u" },
	{ "key", "uu" },
};

WL_EXPORT const struct wl_interface wl_display_interface = {
	"display", 1,
	ARRAY_LENGTH(display_requests), display_requests,
	ARRAY_LENGTH(display_events), display_events,
};

static const struct wl_message compositor_requests[] = {
	{ "create_surface", "n" },
};

WL_EXPORT const struct wl_interface wl_compositor_interface = {
	"compositor", 1,
	ARRAY_LENGTH(compositor_requests), compositor_requests,
	0, NULL,
};

static const struct wl_message drm_requests[] = {
	{ "authenticate", "u" },
	{ "create_buffer", "nuiiuo" },
};

static const struct wl_message drm_events[] = {
	{ "device", "s" },
	{ "authenticated", "" },
};

WL_EXPORT const struct wl_interface wl_drm_interface = {
	"drm", 1,
	ARRAY_LENGTH(drm_requests), drm_requests,
	ARRAY_LENGTH(drm_events), drm_events,
};

static const struct wl_message shm_requests[] = {
	{ "create_buffer", "nhiiuo" },
};

WL_EXPORT const struct wl_interface wl_shm_interface = {
	"shm", 1,
	ARRAY_LENGTH(shm_requests), shm_requests,
	0, NULL,
};

static const struct wl_message buffer_requests[] = {
	{ "destroy", "" },
};

WL_EXPORT const struct wl_interface wl_buffer_interface = {
	"buffer", 1,
	ARRAY_LENGTH(buffer_requests), buffer_requests,
	0, NULL,
};

static const struct wl_message shell_requests[] = {
	{ "move", "oou" },
	{ "resize", "oouu" },
	{ "create_drag", "n" },
	{ "create_selection", "n" },
};

static const struct wl_message shell_events[] = {
	{ "configure", "uuoii" },
};

WL_EXPORT const struct wl_interface wl_shell_interface = {
	"shell", 1,
	ARRAY_LENGTH(shell_requests), shell_requests,
	ARRAY_LENGTH(shell_events), shell_events,
};

static const struct wl_message selection_requests[] = {
	{ "offer", "s" },
	{ "activate", "ou" },
	{ "destroy", "" },
};

static const struct wl_message selection_events[] = {
	{ "send", "sh" },
	{ "cancelled", "" },
};

WL_EXPORT const struct wl_interface wl_selection_interface = {
	"selection", 1,
	ARRAY_LENGTH(selection_requests), selection_requests,
	ARRAY_LENGTH(selection_events), selection_events,
};

static const struct wl_message selection_offer_requests[] = {
	{ "receive", "sh" },
};

static const struct wl_message selection_offer_events[] = {
	{ "offer", "s" },
	{ "keyboard_focus", "o" },
};

WL_EXPORT const struct wl_interface wl_selection_offer_interface = {
	"selection_offer", 1,
	ARRAY_LENGTH(selection_offer_requests), selection_offer_requests,
	ARRAY_LENGTH(selection_offer_events), selection_offer_events,
};

static const struct wl_message drag_requests[] = {
	{ "offer", "s" },
	{ "activate", "oou" },
	{ "destroy", "" },
};

static const struct wl_message drag_events[] = {
	{ "target", "s" },
	{ "finish", "h" },
	{ "reject", "" },
};

WL_EXPORT const struct wl_interface wl_drag_interface = {
	"drag", 1,
	ARRAY_LENGTH(drag_requests), drag_requests,
	ARRAY_LENGTH(drag_events), drag_events,
};

static const struct wl_message drag_offer_requests[] = {
	{ "accept", "us" },
	{ "receive", "h" },
	{ "reject", "" },
};

static const struct wl_message drag_offer_events[] = {
	{ "offer", "s" },
	{ "pointer_focus", "uoiiii" },
	{ "motion", "uiiii" },
	{ "drop", "" },
};

WL_EXPORT const struct wl_interface wl_drag_offer_interface = {
	"drag_offer", 1,
	ARRAY_LENGTH(drag_offer_requests), drag_offer_requests,
	ARRAY_LENGTH(drag_offer_events), drag_offer_events,
};

static const struct wl_message surface_requests[] = {
	{ "destroy", "" },
	{ "attach", "oii" },
	{ "map_toplevel", "" },
	{ "map_transient", "oiiu" },
	{ "map_fullscreen", "" },
	{ "damage", "iiii" },
};

WL_EXPORT const struct wl_interface wl_surface_interface = {
	"surface", 1,
	ARRAY_LENGTH(surface_requests), surface_requests,
	0, NULL,
};

static const struct wl_message input_device_requests[] = {
	{ "attach", "uoii" },
};

static const struct wl_message input_device_events[] = {
	{ "motion", "uiiii" },
	{ "button", "uuu" },
	{ "key", "uuu" },
	{ "pointer_focus", "uoiiii" },
	{ "keyboard_focus", "uoa" },
};

WL_EXPORT const struct wl_interface wl_input_device_interface = {
	"input_device", 1,
	ARRAY_LENGTH(input_device_requests), input_device_requests,
	ARRAY_LENGTH(input_device_events), input_device_events,
};

static const struct wl_message output_events[] = {
	{ "geometry", "iiii" },
};

WL_EXPORT const struct wl_interface wl_output_interface = {
	"output", 1,
	0, NULL,
	ARRAY_LENGTH(output_events), output_events,
};

WL_EXPORT const struct wl_interface wl_visual_interface = {
	"visual", 1,
	0, NULL,
	0, NULL,
};

