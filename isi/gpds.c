/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <errno.h>
#include <string.h>

#include "opcodes/gpds.h"
#include "gpds.h"
#include "modem.h"
#include "debug.h"
#include "gisi/iter.h"
#include "helper.h"

static void gpds_reachable_cb(GIsiClient *client, gboolean alive, uint16_t object, void *opaque) {
	struct isi_cb_data *cbd = opaque;
	isi_subsystem_reachable_cb cb = cbd->callback;
	void *user_data = cbd->data;
	isi_cb_data_free(cbd);

	if (!alive) {
		cb(TRUE, user_data);
		g_warning("unable to bootsrap gprs driver");
		return;
	}

	g_debug("%s (v%03d.%03d)",
		pn_resource_name(g_isi_client_resource(client)),
		g_isi_version_major(client),
		g_isi_version_minor(client));

	cb(FALSE, user_data);
	//g_idle_add(isi_gprs_register, gprs);
}

struct isi_gpds* isi_gpds_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data) {
	struct isi_gpds *nd = calloc(sizeof(struct isi_gpds), 1);
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, data);

	if(!nd || !cbd || !modem->idx)
		goto error;

	nd->client = g_isi_client_create(modem->idx, PN_GPDS);
	if(!nd->client)
		goto error;

	g_isi_verify(nd->client, gpds_reachable_cb, cbd);

	return nd;

	error:
		cb(TRUE, data);
		if(nd)
			free(nd);
		isi_cb_data_free(cbd);
		return NULL;

}

void isi_gpds_destroy(struct isi_gpds *nd) {
	if(!nd)
		return;
	g_isi_client_destroy(nd->client);
	free(nd);
}
