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
		isi_cb_data_free(cbd);
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
	isi_cb_data_free(cbd);
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
		isi_cb_data_free(cbd);
		return;
	}

	if(puklen > SIM_MAX_PUK_LENGTH) {
		cb(SIM_AUTH_ERR_PUK_TOO_LONG, user_data);
		isi_cb_data_free(cbd);
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
	isi_cb_data_free(cbd);
}

static gboolean isi_sim_auth_update_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	isi_sim_auth_cb cb = cbd->callback;
	isi_cb_data_free(cbd);


	if (!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return TRUE;
	}

	switch(msg[0]) {
		case SIM_AUTH_UPDATE_SUCCESS_RESP:
			cb(SIM_AUTH_OK, user_data);
			break;
		case SIM_AUTH_UPDATE_FAIL_RESP:
			switch(msg[1]) {
				case SIM_AUTH_ERROR_INVALID_PW:
					cb(SIM_AUTH_ERR_INVALID, user_data);
					break;
				case SIM_AUTH_ERROR_NEED_PUK:
					cb(SIM_AUTH_ERR_NEED_PUK, user_data);
					break;
				default:
					g_warning("unknown sim auth update response");
					print_package("SIM_PIN_UPDATE", msg, len);
					cb(SIM_AUTH_ERR_UNKNOWN, user_data);
					break;
			}
			break;
		default:
			g_warning("unknown sim auth update response");
			print_package("SIM_PIN_UPDATE", msg, len);
			cb(SIM_AUTH_ERR_UNKNOWN, user_data);
			break;
	}
}

void isi_sim_update_pin(struct isi_sim_auth *nd, char *old_pin, char *new_pin, isi_sim_auth_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	int old_len = strlen(old_pin);
	int new_len = strlen(new_pin);
	int i;

	unsigned char msg[] = {
		SIM_AUTH_UPDATE_REQ, SIM_AUTH_REQ_PIN,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	if(old_len > SIM_MAX_PIN_LENGTH) {
		cb(SIM_AUTH_ERR_PIN_TOO_LONG, user_data);
		isi_cb_data_free(cbd);
		return;
	}

	if(new_len > SIM_MAX_PIN_LENGTH) {
		cb(SIM_AUTH_ERR_PIN_TOO_LONG, user_data);
		isi_cb_data_free(cbd);
		return;
	}

	/* insert the PUK into the request */
	memcpy(msg+2, old_pin, old_len);

	/* insert the PIN into the request */
	memcpy(msg+13, new_pin, new_len);

	if(!cbd)
		goto error;

	if(g_isi_request_make(nd->client, msg, sizeof(msg), SIM_AUTH_TIMEOUT, isi_sim_auth_update_resp_cb, cbd))
		return;

error:
	cb(SIM_AUTH_ERR_UNKNOWN, user_data);
	isi_cb_data_free(cbd);
}

static gboolean isi_sim_auth_status_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = opaque;

	isi_sim_auth_status_cb cb = cbd->callback;
	void *user_data = cbd->data;
	isi_cb_data_free(cbd);


	if(!cb) {
		g_warning("isi_sim_auth_status_resp_cb: no callback defined");
		return TRUE;
	}

	if (!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		cb(SIM_AUTH_STATUS_ERROR, user_data);
		return TRUE;
	}

	switch(msg[1]) {
		case SIM_AUTH_STATUS_RESP_NEED_PIN:
			cb(SIM_AUTH_STATUS_NEED_PIN, user_data);
			break;
		case SIM_AUTH_STATUS_RESP_NEED_PUK:
			cb(SIM_AUTH_STATUS_NEED_PUK, user_data);
			break;
		case SIM_AUTH_STATUS_RESP_RUNNING:
			switch(msg[2]) {
				case SIM_AUTH_STATUS_RESP_RUNNING_AUTHORIZED:
					cb(SIM_AUTH_STATUS_AUTHORIZED, user_data);
					break;
				case SIM_AUTH_STATUS_RESP_RUNNING_UNPROTECTED:
					cb(SIM_AUTH_STATUS_UNPROTECTED, user_data);
					break;
				case SIM_AUTH_STATUS_RESP_RUNNING_NO_SIM:
					cb(SIM_AUTH_STATUS_NO_SIM, user_data);
					break;
				default:
					print_package("SIM Auth Response Package", msg, len);
					cb(SIM_AUTH_STATUS_ERROR, user_data);
					break;
			}
			break;
		case SIM_AUTH_STATUS_RESP_INIT:
			cb(SIM_AUTH_STATUS_INIT, user_data);
			break;
		default:
			print_package("SIM Auth Response Package", msg, len);
			cb(SIM_AUTH_STATUS_ERROR, user_data);
			break;
	}

	return TRUE;
}

void isi_sim_auth_request_status(struct isi_sim_auth *nd, isi_sim_auth_status_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);

	unsigned char msg[] = {
		SIM_AUTH_STATUS_REQ, 0x00, 0x00
	};

	if(!cbd)
		goto error;

	if(g_isi_request_make(nd->client, msg, sizeof(msg), SIM_AUTH_TIMEOUT, isi_sim_auth_status_resp_cb, cbd))
		return;

error:
	cb(SIM_AUTH_STATUS_ERROR, user_data);
	isi_cb_data_free(cbd);
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

	if(msg[3] != SIM_AUTH_IND_OK) {
		switch(msg[3]) {
			case SIM_AUTH_IND_CFG_UNPROTECTED:
				cb(SIM_AUTH_STATUS_UNPROTECTED, user_data);
				break;
			case SIM_AUTH_IND_CFG_PROTECTED:
				cb(SIM_AUTH_STATUS_PROTECTED, user_data);
				break;
			default:
				print_package("SIM Auth Indication Package", msg, len);
				cb(SIM_AUTH_STATUS_ERROR, user_data);
				break;
		}

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
