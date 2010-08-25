/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <errno.h>
#include <string.h>

#include "opcodes/sim.h"
#include "sim.h"
#include "modem.h"
#include "debug.h"
#include "gisi/iter.h"
#include "helper.h"

static gboolean isi_sim_read_hplmn_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_sim *sim = user_data;

	if (!msg) {
		g_debug("ISI client error: %d", g_isi_client_error(client));
		return TRUE;
	}

	if (len < 3 || msg[0] != SIM_NETWORK_INFO_RESP || msg[1] != READ_HPLMN)
		return FALSE;

	if (msg[2] != SIM_SERV_NOTREADY) {
		g_debug("SIM looks OK");
	}

	return TRUE;
}

static void isi_sim_read_hplmn(struct isi_sim *sim) {
	const unsigned char req[] = {
		SIM_NETWORK_INFO_REQ,
		READ_HPLMN, 0
	};

	g_isi_request_make(sim->client, req, sizeof(req), SIM_TIMEOUT, isi_sim_read_hplmn_resp_cb, sim);
}

static void sim_ind_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	isi_subsystem_reachable_cb cb = cbd->callback;

	//if(registered)
	//	return;

	switch (msg[1]) {
		case SIM_ST_PIN:
			g_debug("SIM STATE: SIM_ST_PIN");
			//isi_sim_register(sim);
			break;
		case SIM_ST_INFO:
			g_debug("SIM STATE: SIM_ST_INFO");
			//isi_read_hplmn(sim);
			break;
		default:
			g_debug("SIM STATE: UNKNOWN");
	}

	cb(FALSE, cbd->data);
	isi_cb_data_free(cbd);
}

void sim_reachable_cb(GIsiClient *client, gboolean alive, uint16_t object, void *user_data) {
	struct isi_cb_data *cbd = user_data;
	isi_subsystem_reachable_cb cb = cbd->callback;

	if (!alive) {
		g_critical("Unable to bootstrap SIM driver");
		cb(TRUE, cbd->data);
		isi_cb_data_free(cbd);
		return;
	}

	g_debug("%s (v%03d.%03d) reachable",
		pn_resource_name(g_isi_client_resource(client)),
		g_isi_version_major(client),
		g_isi_version_minor(client));

	g_isi_subscribe(client, SIM_IND, sim_ind_cb, user_data);

	/* Check if SIM is ready */
	isi_sim_read_hplmn(cbd->subsystem);
}

struct isi_sim* isi_sim_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data) {
	struct isi_sim *nd = calloc(sizeof(struct isi_sim), 1);
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, data);

	if(!nd || !cbd || !modem->idx)
		goto error;

	nd->client = g_isi_client_create(modem->idx, PN_SIM);
	if(!nd->client)
		goto error;

	g_isi_verify(nd->client, sim_reachable_cb, cbd);

	return nd;

	error:
		cb(TRUE, data);
		if(nd)
			free(nd);
		isi_cb_data_free(cbd);
		return NULL;
}

void isi_sim_destroy(struct isi_sim *nd) {
	if(!nd)
		return;
	g_isi_client_destroy(nd->client);
	free(nd);
}


static gboolean isi_sim_pin_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	printf("SIM PIN ANSWER: ");
	int i;
	for(i=0; i<len; i++)
		printf("%c", msg[i]);
	printf("\n");
}

void isi_sim_set_pin(struct isi_sim *nd, char *pin, isi_sim_pin_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	int i;
	int len = strlen(pin);

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

	/* insert the PIN into the request */
	for(i=0; i<len; i++)
		msg[2+i] = pin[i];

	if(!cbd)
		goto error;

	if(g_isi_request_make(nd->client, msg, sizeof(msg), SIM_TIMEOUT, isi_sim_pin_resp_cb, cbd))
		return;

error:
	cb(TRUE, SIM_PIN_UNKNOWN_ERROR, user_data);
	g_free(cbd);
}
