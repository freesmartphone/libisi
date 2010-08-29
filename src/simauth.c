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

#include "opcodes/simauth.h"
#include "simauth.h"
#include "modem.h"
#include "debug.h"
#include "gisi/iter.h"
#include "helper.h"

#if 0

	TODO: I guess this is a status request
	TODO: we also need status indication, see network.c to get info how they look like

	/* TODO: sscd uses this magic byte, there is no documentation for it, seems to be some status request */
	unsigned char msg[] = {
		0x11, 0x00, 0x00
	};

	g_debug("Send PN_SIM_AUTH magic packet");
	if(g_isi_request_make(nd->client, msg, sizeof(msg), SIM_TIMEOUT, sim_auth_reachable_cb, cbd))
		return nd;

static gboolean sim_auth_reachable_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	isi_subsystem_reachable_cb cb = cbd->callback;

	g_debug("%s reachable", pn_resource_name(g_isi_client_resource(client)));

	if (!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		if(cb) cb(TRUE, cbd->data);
		isi_cb_data_free(cbd);
		return TRUE;
	}

	unsigned char answer1[] = {
		0x12, 0x02, 0x63, 0x00, 0x00, 0x00, 0x00
	};

	unsigned char answer2[] = {
		0x12, 0x03, 0x63, 0x00, 0x00, 0x00, 0x00
	};

	print_package("SIMAUTH", msg, len);

	if(cb) {
		if(memcmp(msg, answer1, sizeof(answer1)) == 0)
			cb(FALSE, cbd->data);
		else if(memcmp(msg, answer2, sizeof(answer2)) == 0)
			cb(FALSE, cbd->data);
		else
			cb(TRUE, cbd->data);
	}

	//isi_cb_data_free(cbd);
}
#endif


struct isi_sim_auth* isi_sim_auth_create(struct isi_modem *modem) {
	struct isi_sim_auth *nd = calloc(sizeof(struct isi_sim_auth), 1);

	if(!nd)
		return NULL;

	if(modem->idx)
		nd->client = g_isi_client_create(modem->idx, PN_SIM_AUTH);

	if(nd->client)
		return nd;

	free(nd);
	return NULL;
}

void isi_sim_auth_destroy(struct isi_sim_auth *nd) {
	if(!nd)
		return;
	g_isi_client_destroy(nd->client);
	free(nd);
}

static gboolean isi_sim_auth_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	isi_sim_auth_cb cb = cbd->callback;


	if (!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return TRUE;
	}

	if(msg[0] == SIM_AUTH_FAIL_RESP) {
		switch(msg[1]) {
			case SIM_AUTH_ERROR_INVALID_PW:
				cb(SIM_AUTH_ERR_INVALID, cbd->data);
				break;
			case SIM_AUTH_ERROR_NEED_PUK:
				cb(SIM_AUTH_ERR_NEED_PUK, cbd->data);
				break;
			default:
				g_warning("UNKNOWN SIM AUTH RESPONSE: 0x080x%02x", msg[1]);
				cb(SIM_AUTH_ERR_UNKNOWN, cbd->data);
				break;
		}
	} else if(msg[0] == SIM_AUTH_SUCCESS_RESP) {
		if(msg[1] == 0x63)
			cb(SIM_AUTH_OK, cbd->data);
		else {
			g_warning("UNKNOWN SIM AUTH RESPONSE: 0x080x%02x", msg[1]);
			cb(SIM_AUTH_ERR_UNKNOWN, cbd->data);
		}
	} else {
		g_warning("UNKNOWN SIM AUTH RESPONSE");
		print_package("SIM_PIN", msg, len);
		cb(SIM_AUTH_ERR_UNKNOWN, cbd->data);
	}

}

void isi_sim_auth_set_pin(struct isi_sim_auth *nd, char *pin, isi_sim_auth_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	int len = strlen(pin);
	int i;

	unsigned char msg[] = {
		SIM_AUTH_REQ, SIM_AUTH_REQ_PIN,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

	if(len > SIM_MAX_PIN_LENGTH) {
		cb(SIM_AUTH_ERR_PIN_TOO_LONG, user_data);
		g_free(cbd);
		return;
	}

	/* insert the PIN into the request */
	memcpy(msg+2, pin, len);

	if(!cbd)
		goto error;

	if(g_isi_request_make(nd->client, msg, sizeof(msg), SIM_AUTH_TIMEOUT, isi_sim_auth_resp_cb, cbd))
		return;

error:
	cb(SIM_AUTH_ERR_UNKNOWN, user_data);
	g_free(cbd);
}

void isi_sim_auth_set_puk(struct isi_sim_auth *nd, char *puk, char *pin, isi_sim_auth_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	int puklen = strlen(puk);
	int pinlen = strlen(pin);
	int i;

	unsigned char msg[] = {
		SIM_AUTH_REQ, SIM_AUTH_REQ_PUK,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

	if(pinlen > SIM_MAX_PIN_LENGTH) {
		cb(SIM_AUTH_ERR_PIN_TOO_LONG, user_data);
		g_free(cbd);
		return;
	}

	if(puklen > SIM_MAX_PUK_LENGTH) {
		cb(SIM_AUTH_ERR_PUK_TOO_LONG, user_data);
		g_free(cbd);
		return;
	}

	/* insert the PUK into the request */
	memcpy(msg+2, puk, puklen);

	/* insert the PIN into the request */
	memcpy(msg+13, pin, pinlen);

	if(!cbd)
		goto error;

	if(g_isi_request_make(nd->client, msg, sizeof(msg), SIM_AUTH_TIMEOUT, isi_sim_auth_resp_cb, cbd))
		return;

error:
	cb(SIM_AUTH_ERR_UNKNOWN, user_data);
	g_free(cbd);
}

void sim_auth_ind_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = opaque;
	struct isi_network *nd = cbd->subsystem;
	isi_sim_auth_status_cb cb = cbd->callback;
	void *user_data = cbd->data;

	if(!msg || len < 3 || msg[0] != SIM_AUTH_STATUS_IND) {
		cb(SIM_AUTH_STATUS_ERROR, user_data);
		return;
	}

	switch(msg[1]) {
		case SIM_AUTH_IND_NEED_AUTH:
			switch(msg[2]) {
				case SIM_AUTH_IND_PIN:
					cb(SIM_AUTH_STATUS_NEED_PIN, user_data);
					break;
				case SIM_AUTH_IND_PUK:
					cb(SIM_AUTH_STATUS_NEED_PUK, user_data);
					break;
				default:
					print_package("SIM Auth Indication Package", msg, len);
					break;
			}
			break;
		case SIM_AUTH_IND_NEED_NO_AUTH:
			cb(SIM_AUTH_STATUS_NEED_NONE, user_data);
			break;
		case SIM_AUTH_IND_VALID:
			switch(msg[2]) {
				case SIM_AUTH_IND_PIN:
					cb(SIM_AUTH_STATUS_VALID_PIN, user_data);
					break;
				case SIM_AUTH_IND_PUK:
					cb(SIM_AUTH_STATUS_VALID_PUK, user_data);
					break;
				default:
					print_package("SIM Auth Indication Package", msg, len);
					break;
			}
			break;
		case SIM_AUTH_IND_INVALID:
			switch(msg[2]) {
				case SIM_AUTH_IND_PIN:
					cb(SIM_AUTH_STATUS_INVALID_PIN, user_data);
					break;
				case SIM_AUTH_IND_PUK:
					cb(SIM_AUTH_STATUS_INVALID_PUK, user_data);
					break;
				default:
					print_package("SIM Auth Indication Package", msg, len);
					break;
			}
			break;
		case SIM_AUTH_IND_AUTHORIZED:
			cb(SIM_AUTH_STATUS_AUTHORIZED, user_data);
			break;
		default:
			print_package("SIM Auth Indication Package", msg, len);
			break;
	}

}

void isi_sim_auth_subscribe_status(struct isi_sim_auth *nd, isi_sim_auth_status_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	if(!cbd || g_isi_subscribe(nd->client, SIM_AUTH_STATUS_IND, sim_auth_ind_cb, cbd)) {
		isi_cb_data_free(cbd);
		cb(SIM_AUTH_STATUS_ERROR, user_data);
	}
}

void isi_sim_auth_unsubscribe_status(struct isi_sim_auth *nd) {
	g_isi_unsubscribe(nd->client, SIM_AUTH_STATUS_IND);
}
