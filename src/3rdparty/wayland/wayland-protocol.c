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

static const struct wl_message wl_display_requests[] = {
	{ "bind", "usu" },
	{ "sync", "u" },
	{ "frame", "ou" },
};

static const struct wl_message wl_display_events[] = {
	{ "invalid_object", "u" },
	{ "invalid_method", "uu" },
	{ "no_memory", "" },
	{ "global", "nsu" },
	{ "range", "u" },
	{ "key", "uu" },
};

WL_EXPORT const struct wl_interface wl_display_interface = {
	"wl_display", 1,
	ARRAY_LENGTH(wl_display_requests), wl_display_requests,
	ARRAY_LENGTH(wl_display_events), wl_display_events,
};

static const struct wl_message wl_compositor_requests[] = {
	{ "create_surface", "n" },
};

WL_EXPORT const struct wl_interface wl_compositor_interface = {
	"wl_compositor", 1,
	ARRAY_LENGTH(wl_compositor_requests), wl_compositor_requests,
	0, NULL,
};

static const struct wl_message wl_shm_requests[] = {
	{ "create_buffer", "nhiiuo" },
};

WL_EXPORT const struct wl_interface wl_shm_interface = {
	"wl_shm", 1,
	ARRAY_LENGTH(wl_shm_requests), wl_shm_requests,
	0, NULL,
};

static const struct wl_message wl_buffer_requests[] = {
	{ "damage", "iiii" },
	{ "destroy", "" },
};

WL_EXPORT const struct wl_interface wl_buffer_interface = {
	"wl_buffer", 1,
	ARRAY_LENGTH(wl_buffer_requests), wl_buffer_requests,
	0, NULL,
};

static const struct wl_message wl_shell_requests[] = {
	{ "move", "oou" },
	{ "resize", "oouu" },
	{ "create_drag", "n" },
	{ "create_selection", "n" },
};

static const struct wl_message wl_shell_events[] = {
	{ "configure", "uuoii" },
};

WL_EXPORT const struct wl_interface wl_shell_interface = {
	"wl_shell", 1,
	ARRAY_LENGTH(wl_shell_requests), wl_shell_requests,
	ARRAY_LENGTH(wl_shell_events), wl_shell_events,
};

static const struct wl_message wl_selection_requests[] = {
	{ "offer", "s" },
	{ "activate", "ou" },
	{ "destroy", "" },
};

static const struct wl_message wl_selection_events[] = {
	{ "send", "sh" },
	{ "cancelled", "" },
};

WL_EXPORT const struct wl_interface wl_selection_interface = {
	"wl_selection", 1,
	ARRAY_LENGTH(wl_selection_requests), wl_selection_requests,
	ARRAY_LENGTH(wl_selection_events), wl_selection_events,
};

static const struct wl_message wl_selection_offer_requests[] = {
	{ "receive", "sh" },
};

static const struct wl_message wl_selection_offer_events[] = {
	{ "offer", "s" },
	{ "keyboard_focus", "o" },
};

WL_EXPORT const struct wl_interface wl_selection_offer_interface = {
	"wl_selection_offer", 1,
	ARRAY_LENGTH(wl_selection_offer_requests), wl_selection_offer_requests,
	ARRAY_LENGTH(wl_selection_offer_events), wl_selection_offer_events,
};

static const struct wl_message wl_drag_requests[] = {
	{ "offer", "s" },
	{ "activate", "oou" },
	{ "destroy", "" },
};

static const struct wl_message wl_drag_events[] = {
	{ "target", "s" },
	{ "finish", "h" },
	{ "reject", "" },
};

WL_EXPORT const struct wl_interface wl_drag_interface = {
	"wl_drag", 1,
	ARRAY_LENGTH(wl_drag_requests), wl_drag_requests,
	ARRAY_LENGTH(wl_drag_events), wl_drag_events,
};

static const struct wl_message wl_drag_offer_requests[] = {
	{ "accept", "us" },
	{ "receive", "h" },
	{ "reject", "" },
};

static const struct wl_message wl_drag_offer_events[] = {
	{ "offer", "s" },
	{ "pointer_focus", "uoiiii" },
	{ "motion", "uiiii" },
	{ "drop", "" },
};

WL_EXPORT const struct wl_interface wl_drag_offer_interface = {
	"wl_drag_offer", 1,
	ARRAY_LENGTH(wl_drag_offer_requests), wl_drag_offer_requests,
	ARRAY_LENGTH(wl_drag_offer_events), wl_drag_offer_events,
};

static const struct wl_message wl_surface_requests[] = {
	{ "destroy", "" },
	{ "attach", "oii" },
	{ "map_toplevel", "" },
	{ "map_transient", "oiiu" },
	{ "map_fullscreen", "" },
	{ "damage", "iiii" },
};

WL_EXPORT const struct wl_interface wl_surface_interface = {
	"wl_surface", 1,
	ARRAY_LENGTH(wl_surface_requests), wl_surface_requests,
	0, NULL,
};

static const struct wl_message wl_input_device_requests[] = {
	{ "attach", "uoii" },
};

static const struct wl_message wl_input_device_events[] = {
	{ "motion", "uiiii" },
	{ "button", "uuu" },
	{ "key", "uuu" },
	{ "pointer_focus", "uoiiii" },
	{ "keyboard_focus", "uoa" },
};

WL_EXPORT const struct wl_interface wl_input_device_interface = {
	"wl_input_device", 1,
	ARRAY_LENGTH(wl_input_device_requests), wl_input_device_requests,
	ARRAY_LENGTH(wl_input_device_events), wl_input_device_events,
};

static const struct wl_message wl_output_events[] = {
	{ "geometry", "iiii" },
};

WL_EXPORT const struct wl_interface wl_output_interface = {
	"wl_output", 1,
	0, NULL,
	ARRAY_LENGTH(wl_output_events), wl_output_events,
};

WL_EXPORT const struct wl_interface wl_visual_interface = {
	"wl_visual", 1,
	0, NULL,
	0, NULL,
};

