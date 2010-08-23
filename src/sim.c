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
	isi_sim_read_hplmn(cbd->data);
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
