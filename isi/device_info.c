/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <errno.h>
#include <string.h>

#include "opcodes/info.h"
#include "device_info.h"
#include "modem.h"
#include "debug.h"
#include "gisi/iter.h"
#include "helper.h"

void device_info_reachable_cb(GIsiClient *client, gboolean alive, uint16_t object, void *user_data) {
	struct isi_cb_data *cbd = user_data;
	isi_subsystem_reachable_cb cb = cbd->callback;

	if (!alive) {
		g_critical("Unable to bootstrap device info driver");
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

struct isi_device_info* isi_device_info_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data) {
	struct isi_device_info *nd = calloc(sizeof(struct isi_device_info), 1);
	struct isi_cb_data *cbd = isi_cb_data_new(NULL, cb, data);

	if(!nd || !cbd || !modem->idx)
		goto error;

	nd->client = g_isi_client_create(modem->idx, PN_PHONE_INFO);
	if(!nd->client)
		goto error;

	g_isi_verify(nd->client, device_info_reachable_cb, cbd);

	return nd;

	error:
		cb(TRUE, data);
		if(nd)
			free(nd);
		isi_cb_data_free(cbd);
		return NULL;
}

void isi_device_info_destroy(struct isi_device_info *nd) {
	if(!nd)
		return;
	g_isi_client_destroy(nd->client);
	free(nd);
}

static gboolean isi_device_info_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = opaque;
	isi_device_info_cb cb = cbd->callback;

	GIsiSubBlockIter iter;
	char *info = NULL;
	guint8 chars;

	if (!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	if (len < 3) {
		g_debug("truncated message");
		return FALSE;
	}

	if (msg[0] != INFO_PRODUCT_INFO_READ_RESP && msg[0] != INFO_VERSION_READ_RESP
		&& msg[0] != INFO_SERIAL_NUMBER_READ_RESP)
		return FALSE;

	if (msg[1] != INFO_OK) {
		g_debug("request failed: %s", isi_device_info_isi_cause_name(msg[1]));
		goto error;
	}

	for (g_isi_sb_iter_init(&iter, msg, len, 3);
		g_isi_sb_iter_is_valid(&iter);
		g_isi_sb_iter_next(&iter)) {

		switch (g_isi_sb_iter_get_id(&iter)) {

		case INFO_SB_PRODUCT_INFO_MANUFACTURER:
		case INFO_SB_PRODUCT_INFO_NAME:
		case INFO_SB_MCUSW_VERSION:
		case INFO_SB_SN_IMEI_PLAIN:

			if (g_isi_sb_iter_get_len(&iter) < 5
				|| !g_isi_sb_iter_get_byte(&iter, &chars, 3)
				|| !g_isi_sb_iter_get_latin_tag(&iter,
							&info, chars, 4))
				goto error;

			cb(FALSE, info, cbd->data);
			g_free(info);

			g_free(cbd);
			return TRUE;

		default:
			g_debug("skipping: %s (%zu bytes)", isi_device_info_subblock_name(g_isi_sb_iter_get_id(&iter)), g_isi_sb_iter_get_len(&iter));
			break;
		}
	}

error:
	cb(TRUE, "", cbd->data);
	g_free(cbd);
	return TRUE;
}

void isi_device_info_query_manufacturer(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);

	const unsigned char msg[] = {
		INFO_PRODUCT_INFO_READ_REQ,
		INFO_PRODUCT_MANUFACTURER
	};

	if (!cbd)
		goto error;

	if (g_isi_request_make(nd->client, msg, sizeof(msg), INFO_TIMEOUT, isi_device_info_resp_cb, cbd))
		return;

error:
	cb(TRUE, "", user_data);
	g_free(cbd);
}

void isi_device_info_query_model(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);

	const unsigned char msg[] = {
		INFO_PRODUCT_INFO_READ_REQ,
		INFO_PRODUCT_NAME
	};

	if (!cbd)
		goto error;

	if (g_isi_request_make(nd->client, msg, sizeof(msg), INFO_TIMEOUT, isi_device_info_resp_cb, cbd))
		return;

error:
	cb(TRUE, "", user_data);
	g_free(cbd);
}

void isi_device_info_query_revision(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);

	const unsigned char msg[] = {
		INFO_VERSION_READ_REQ,
		0x00, INFO_MCUSW,
		0x00, 0x00, 0x00, 0x00
	};

	if (!cbd)
		goto error;

	if (g_isi_request_make(nd->client, msg, sizeof(msg), INFO_TIMEOUT, isi_device_info_resp_cb, cbd))
		return;

error:
	cb(TRUE, "", user_data);
	g_free(cbd);
}

void isi_device_info_query_serial(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);

	const unsigned char msg[] = {
		INFO_SERIAL_NUMBER_READ_REQ,
		INFO_SN_IMEI_PLAIN
	};

	if (!cbd)
		goto error;

	if (g_isi_request_make(nd->client, msg, sizeof(msg), INFO_TIMEOUT, isi_device_info_resp_cb, cbd))
		return;

error:
	cb(TRUE, "", user_data);
	g_free(cbd);
}
