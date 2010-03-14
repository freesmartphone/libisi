/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <stdio.h>
#include <glib.h>
#include <errno.h>

#include "gisi/modem.h"
#include "opcodes/mtc.h"

#include "debug.h"
#include "modem.h"

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

	isi->powerstatus(msg[1] != MTC_POWER_OFF);
}

bool mtc_query_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_modem *isi = opaque;

	if(!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return true;
	}

	if(len < 3 || msg[0] != MTC_STATE_QUERY_RESP)
		return false;

	g_debug("current modem state: %s (0x%02X)", mtc_modem_state_name(msg[1]), msg[1]);
	g_debug("target modem state: %s (0x%02X)", mtc_modem_state_name(msg[2]), msg[2]);

	isi->powerstatus(msg[1] != MTC_POWER_OFF);
	return true;
}

void reachable_cb(GIsiClient *client, bool alive, uint16_t object, void *opaque) {
	const unsigned char msg[] = {
		MTC_STATE_QUERY_REQ,
		0x00, 0x00 /* Filler */
	};

	if(!alive) {
		g_debug("Unable to bootstrap mtc driver");
		return;
	}

	g_debug("%s (v.%03d.%03d) reachable",
		pn_resource_name(g_isi_client_resource(client)),
		g_isi_version_major(client),
		g_isi_version_minor(client));

	g_isi_subscribe(client, MTC_STATE_INFO_IND, mtc_state_cb, opaque);
	g_isi_request_make(client, msg, sizeof(msg), MTC_TIMEOUT, mtc_query_cb, opaque);
}

bool mtc_power_on_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_modem *isi = opaque;

	if(!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return true;
	}

	if(len < 2 || msg[0] != MTC_POWER_ON_RESP)
		return false;

	if(msg[1] == MTC_OK)
		isi->powerstatus(true);

	return true;
}

bool mtc_power_off_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_modem *isi = opaque;

	if(!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return true;
	}

	if(len < 2 || msg[0] != MTC_POWER_OFF_RESP)
		return false;

	if(msg[1] == MTC_OK)
		isi->powerstatus(false);

	return true;
}

int isi_modem_create(struct isi_modem *modem) {
	modem->client = g_isi_client_create(modem->idx, PN_MTC);
	if (!modem->client)
		return -ENOMEM;

	g_isi_verify(modem->client, reachable_cb, modem);

	return 0;
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
