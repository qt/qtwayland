#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

static const struct wl_message wl_windowmanager_requests[] = {
	{ "map_client_to_process", "u", NULL },
	{ "authenticate_with_token", "s", NULL },
	{ "update_generic_property", "osa", NULL },
};

static const struct wl_message wl_windowmanager_events[] = {
	{ "client_onscreen_visibility", "i", NULL },
	{ "set_screen_rotation", "oi", NULL },
	{ "set_generic_property", "osa", NULL },
};

WL_EXPORT const struct wl_interface wl_windowmanager_interface = {
	"wl_windowmanager", 1,
	ARRAY_LENGTH(wl_windowmanager_requests), wl_windowmanager_requests,
	ARRAY_LENGTH(wl_windowmanager_events), wl_windowmanager_events,
};

