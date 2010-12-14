/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <stdio.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "gisi/modem.h"
#include "opcodes/mtc.h"

#include "debug.h"
#include "modem.h"
#include <netlink/netlink.h>
#include "helper.h"

void mtc_state_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_modem *isi = opaque;

	if(!msg) {
		g_print("ISI client error: %d", g_isi_client_error(client));
		return;
	}

	if(len < 3 || msg[0] != MTC_STATE_INFO_IND)
		return;

	g_debug("current modem state: %s (0x%02X)", mtc_modem_state_name(msg[1]), msg[1]);
	g_debug("target modem state: %s (0x%02X)", mtc_modem_state_name(msg[2]), msg[2]);

	isi->power = (msg[1] != MTC_POWER_OFF);
	if(isi->powerstatus)
		isi->powerstatus(msg[1] != MTC_POWER_OFF, isi->user_data);
}

gboolean mtc_query_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_modem *isi = opaque;

	if(!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return TRUE;
	}

	if(len < 3 || msg[0] != MTC_STATE_QUERY_RESP)
		return FALSE;

	g_debug("current modem state: %s (0x%02X)", mtc_modem_state_name(msg[1]), msg[1]);
	g_debug("target modem state: %s (0x%02X)", mtc_modem_state_name(msg[2]), msg[2]);

	isi->power = (msg[1] != MTC_POWER_OFF);
	return TRUE;
}

void modem_reachable_cb(GIsiClient *client, gboolean alive, uint16_t object, void *opaque) {
	struct isi_cb_data *cbd = opaque;
	isi_subsystem_reachable_cb cb = cbd->callback;
	struct isi_modem *modem = cbd->subsystem;
	void * user_data = cbd->data;
	isi_cb_data_free(opaque);

	const unsigned char msg[] = {
		MTC_STATE_QUERY_REQ,
		0x00, 0x00 /* Filler */
	};

	if(!alive) {
		g_warning("Unable to bootstrap mtc driver");
		cb(TRUE, user_data);
		return;
	}

	g_debug("%s (v.%03d.%03d) reachable",
		pn_resource_name(g_isi_client_resource(client)),
		g_isi_version_major(client),
		g_isi_version_minor(client));

	cb(FALSE, user_data);

	g_isi_subscribe(client, MTC_STATE_INFO_IND, mtc_state_cb, modem);
	g_isi_request_make(client, msg, sizeof(msg), MTC_TIMEOUT, mtc_query_cb, modem);
}

gboolean mtc_power_on_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_modem *isi = opaque;

	if(!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return TRUE;
	}

	if(len < 2 || msg[0] != MTC_POWER_ON_RESP)
		return FALSE;

	if(msg[1] == MTC_OK) {
		isi->power = TRUE;
		if(isi->powerstatus)
				isi->powerstatus(TRUE, isi->user_data);
	}

	return TRUE;
}

gboolean mtc_power_off_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_modem *isi = opaque;

	if(!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return TRUE;
	}

	if(len < 2 || msg[0] != MTC_POWER_OFF_RESP)
		return FALSE;

	if(msg[1] == MTC_OK) {
		isi->power = FALSE;
		if(isi->powerstatus)
			isi->powerstatus(FALSE, isi->user_data);
	}

	return TRUE;
}

static void netlink_status_cb(GIsiModem *idx, GPhonetLinkState state, char const *iface, void *data) {
	struct isi_cb_data *cbd = data;
	isi_subsystem_reachable_cb cb = cbd->callback;
	struct isi_modem *modem = cbd->subsystem;
	void * user_data = cbd->data;

	g_debug("Phonet: %s is %s, idx=%p", iface, state == PN_LINK_UP ? "up" : "down", idx);

	if(state == PN_LINK_UP) {
		modem->idx = idx;
		modem->client = g_isi_client_create(idx, PN_MTC);

		if(!modem->client) {
			cb(TRUE, user_data);
			isi_cb_data_free(cbd);
			return;
		}

		g_isi_verify(modem->client, modem_reachable_cb, cbd);
	} else {
		g_isi_client_destroy(modem->client);
		modem->client = NULL;
		modem->idx = NULL;
		modem->status = FALSE;
		cb(TRUE, user_data);
	}
}

struct isi_modem* isi_modem_create(char *interface, isi_subsystem_reachable_cb cb, void *user_data) {
	struct isi_modem *modem = malloc(sizeof(struct isi_modem));
	struct isi_cb_data *cbd = isi_cb_data_new(modem, cb, user_data);
	int error;

	if(!modem || !cbd)
		goto error;

	modem->idx = g_isi_modem_by_name(interface);
	modem->link = g_pn_netlink_start(modem->idx, netlink_status_cb, cbd);

	if(!modem->link)
		goto error;
	
	error = g_pn_netlink_set_address(modem->idx, PN_DEV_SOS);
	if(error && error != -EEXIST)
		g_debug("g_pn_netlink_set_address: %s\n", strerror(-error));

	error = g_pn_netlink_add_route(modem->idx, PN_DEV_HOST);
	if(error && error != -ENOTSUP)
		g_debug("g_pn_netlink_add_route: %s\n", strerror(-error));

	return modem;

	error:
		if(modem)
			free(modem);
		isi_cb_data_free(cbd);
		cb(TRUE, user_data);
		return NULL;
}

void isi_modem_destroy(struct isi_modem *modem) {
	g_isi_client_destroy(modem->client);
	free(modem);
}

void isi_modem_set_powerstatus_notification(struct isi_modem *modem, isi_powerstatus_cb cb, void *user_data) {
	modem->powerstatus = cb;
	modem->user_data = user_data;
}

gboolean isi_modem_get_powerstatus(struct isi_modem *modem) {
	return modem->power;
}

int isi_modem_enable(struct isi_modem *modem) {
	const unsigned char msg[] = {
		MTC_POWER_ON_REQ,
		0x00, 0x00 /* Filler */
	};

	if (!g_isi_request_make(modem->client, msg, sizeof(msg), MTC_TIMEOUT, mtc_power_on_cb, modem))
		return -EINVAL;

	return -EINPROGRESS;
}

int isi_modem_disable(struct isi_modem *modem) {
	const unsigned char msg[] = {
		MTC_POWER_OFF_REQ,
		0x00, 0x00 /* Filler */
	};

	if (!g_isi_request_make(modem->client, msg, sizeof(msg), MTC_TIMEOUT, mtc_power_off_cb, modem))
		return -EINVAL;

	return -EINPROGRESS;
}
