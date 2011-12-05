#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_buffer_interface;

static const struct wl_interface *types[] = {
	NULL,
	NULL,
	&wl_buffer_interface,
	NULL,
	NULL,
	NULL,
};

static const struct wl_message wl_xcomposite_requests[] = {
	{ "create_buffer", "nuii", types + 2 },
};

static const struct wl_message wl_xcomposite_events[] = {
	{ "root", "su", types + 0 },
};

WL_EXPORT const struct wl_interface wl_xcomposite_interface = {
	"wl_xcomposite", 1,
	ARRAY_LENGTH(wl_xcomposite_requests), wl_xcomposite_requests,
	ARRAY_LENGTH(wl_xcomposite_events), wl_xcomposite_events,
};

