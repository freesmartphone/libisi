/*
 * Copyright (C) 2010, Sebastian Reichel <sre@ring0.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <errno.h>
#include <string.h>

#include "opcodes/gps.h"
#include "gps.h"
#include "modem.h"
#include "debug.h"
#include "gisi/iter.h"
#include "helper.h"

static void gps_status_ind_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = opaque;
	struct isi_gps *nd = cbd->subsystem;
	isi_gps_status_cb cb = cbd->callback;
	void *user_data = cbd->data;

	if(!cb) {
		g_warning("no callback defined!");
		return;
	}

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	/* TODO: analyze gps status pkg */

	error:
		cb(ISI_GPS_STATUS_ERROR, user_data);

}


void isi_gps_status_subscribe(struct isi_gps *nd, isi_gps_status_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	if(!cbd || g_isi_subscribe(nd->client, GPS_STATUS_IND, gps_status_ind_cb, cbd)) {
		isi_cb_data_free(cbd);
		cb(ISI_GPS_STATUS_ERROR, user_data);
	}
}

void isi_gps_status_unsubscribe(struct isi_gps *nd) {
	g_isi_unsubscribe(nd->client, GPS_STATUS_IND);
}

static void gps_data_ind_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = opaque;
	struct isi_gps *nd = cbd->subsystem;
	isi_gps_data_cb cb = cbd->callback;
	void *user_data = cbd->data;

	if(!cb) {
		g_warning("no callback defined!");
		return;
	}

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	/* TODO: analyze gps data pkg */

	error:
		cb(TRUE, NULL, user_data);
}

void isi_gps_data_subscribe(struct isi_gps *nd, isi_gps_data_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	if(!cbd || g_isi_subscribe(nd->client, GPS_DATA_IND, gps_data_ind_cb, cbd)) {
		isi_cb_data_free(cbd);
		cb(TRUE, 0, user_data);
	}
}

void isi_gps_data_unsubscribe(struct isi_gps *nd) {
	g_isi_unsubscribe(nd->client, GPS_DATA_IND);
}

void gps_reachable_cb(GIsiClient *client, gboolean alive, uint16_t object, void *user_data) {
	struct isi_cb_data *cbd = user_data;
	isi_subsystem_reachable_cb cb = cbd->callback;

	if (!alive) {
		g_critical("Unable to bootstrap gps driver");
		cb(TRUE, cbd->data);
		return;
	}

	g_debug("%s (v%03d.%03d) reachable",
		pn_resource_name(g_isi_client_resource(client)),
		g_isi_version_major(client),
		g_isi_version_minor(client));

	cb(FALSE, cbd->data);
	isi_cb_data_free(cbd);
}

struct isi_gps* isi_gps_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *user_data) {
	struct isi_gps *nd = calloc(sizeof(struct isi_gps), 1);
	struct isi_cb_data *cbd = isi_cb_data_new(NULL, cb, user_data);

	if(!nd || !cbd || !modem->idx)
		goto error;

	nd->client = g_isi_client_create(modem->idx, PN_NETWORK);
	if(!nd->client)
		goto error;

	g_isi_verify(nd->client, gps_reachable_cb, cbd);

	return nd;

	error:
		cb(TRUE, user_data);
		if(nd)
			free(nd);
		isi_cb_data_free(cbd);
		return NULL;
}

void isi_gps_destroy(struct isi_gps *nd) {
	if(!nd)
		return;
	g_isi_client_destroy(nd->client);
	free(nd);
}
