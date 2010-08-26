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

#include "opcodes/sim.h"
#include "sim.h"
#include "modem.h"
#include "debug.h"
#include "gisi/iter.h"
#include "helper.h"

void sim_auth_reachable_cb(GIsiClient *client, gboolean alive, uint16_t object, void *user_data) {
	struct isi_cb_data *cbd = user_data;
	isi_subsystem_reachable_cb cb = cbd->callback;

	if (!alive) {
		g_critical("Unable to bootstrap SIM Authentication driver");
		cb(TRUE, cbd->data);
		isi_cb_data_free(cbd);
		return;
	}

	g_debug("%s (v%03d.%03d) reachable",
		pn_resource_name(g_isi_client_resource(client)),
		g_isi_version_major(client),
		g_isi_version_minor(client));

	cb(FALSE, cbd->data);
}


struct isi_sim_auth* isi_sim_auth_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data) {
	struct isi_sim_auth *nd = calloc(sizeof(struct isi_sim_auth), 1);
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, data);

	if(!nd || !cbd || !modem->idx)
		goto error;

	nd->client = g_isi_client_create(modem->idx, PN_SIM_AUTH);
	if(!nd->client)
		goto error;

	g_isi_verify(nd->client, sim_auth_reachable_cb, cbd);

	return nd;

	error:
		cb(TRUE, data);
		if(nd)
			free(nd);
		isi_cb_data_free(cbd);
		return NULL;
}

void isi_sim_auth_destroy(struct isi_sim_auth *nd) {
	if(!nd)
		return;
	g_isi_client_destroy(nd->client);
	free(nd);
}

static gboolean isi_sim_pin_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;

	if (!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return TRUE;
	}

	printf("OBJECT: %d\n", object);
	printf("SIM PIN ANSWER: ");
	int i;
	for(i=0; i<len; i++)
		printf("%02x", msg[i]);
	printf("\n");
}

void isi_sim_auth_set_pin(struct isi_sim_auth *nd, char *pin, isi_sim_pin_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	int i;
	int len = strlen(pin);

	printf("isi_sim_set_pin...\n");

	unsigned char msg[] = {
		SIM_AUTHENTICATION_REQ, 0x02, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	/* maximum PIN length (at least according to Nokia's GUI */
	if(len > 8) {
		cb(TRUE, SIM_PIN_TOO_LONG, user_data);
		g_free(cbd);
		return;
	}

	printf("Package: ");

	/* insert the PIN into the request */
	for(i=0; i<len; i++)
		msg[2+i] = pin[i];

	for(i=0; i<24; i++)
		printf("%02x", msg[i]);
	printf("\n");

	if(!cbd)
		goto error;

	if(g_isi_request_make(nd->client, msg, sizeof(msg), SIM_TIMEOUT, isi_sim_pin_resp_cb, cbd))
		return;

error:
	cb(TRUE, SIM_PIN_UNKNOWN_ERROR, user_data);
	g_free(cbd);
}
