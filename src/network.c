/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <errno.h>

#include "opcodes/network.h"
#include "network.h"
#include "debug.h"
#include "gisi/iter.h"

gboolean decode_reg_status(struct network_data *nd, const guint8 *msg, size_t len, struct network_status *st) {
	enum net_reg_status *status = &st->status;
	guint16 *lac = &st->lac;
	guint16 *ci = &st->cid;
	enum net_technology *tech = &st->technology;

	GIsiSubBlockIter iter;

	g_isi_sb_iter_init(&iter, msg, len, 0);

	while (g_isi_sb_iter_is_valid(&iter)) {

		switch (g_isi_sb_iter_get_id(&iter)) {

			case NET_REG_INFO_COMMON: {
				guint8 byte = 0;

				if (!g_isi_sb_iter_get_byte(&iter, &byte, 2))
					return FALSE;

				if (!g_isi_sb_iter_get_byte(&iter,
							&nd->last_reg_mode, 3))
					return FALSE;

				*status = byte;

				/* FIXME: decode alpha tag(s) */
				break;
			}

			case NET_GSM_REG_INFO: {
				guint16 word = 0;
				guint32 dword = 0;
				guint8 egprs = 0;
				guint8 hsdpa = 0;
				guint8 hsupa = 0;

				if (!g_isi_sb_iter_get_word(&iter, &word, 2) ||
					!g_isi_sb_iter_get_dword(&iter, &dword, 4) ||
					!g_isi_sb_iter_get_byte(&iter, &egprs, 17) ||
					!g_isi_sb_iter_get_byte(&iter, &hsdpa, 20) ||
					!g_isi_sb_iter_get_byte(&iter, &hsupa, 21))
					return FALSE;

				*ci = (int)dword & 0x0000FFFF;
				*lac = (int)word;

				switch (nd->rat) {

				case NET_GSM_RAT:

					*tech = 0;
					if (nd->gsm_compact)
						*tech = 1;
					else if (egprs)
						*tech = 3;
					break;

				case NET_UMTS_RAT:

					*tech = 2;
					if (hsdpa)
						*tech = 4;
					if (hsupa)
						*tech = 5;
					if (hsdpa && hsupa)
						*tech = 6;
					break;

				default:
					*tech = 0;
				}

				break;
			}

			default:
				g_debug("Skipping sub-block: %s (%zu bytes)",
					net_subblock_name(g_isi_sb_iter_get_id(&iter)),
					g_isi_sb_iter_get_len(&iter));
				break;
		}

		g_isi_sb_iter_next(&iter);
	}

	return TRUE;
}

static inline int isi_status_to_at_status(guint8 status) {
	switch (status) {

		case NET_REG_STATUS_NOSERV:
		case NET_REG_STATUS_NOSERV_NOTSEARCHING:
		case NET_REG_STATUS_NOSERV_NOSIM:
		case NET_REG_STATUS_POWER_OFF:
		case NET_REG_STATUS_NSPS:
		case NET_REG_STATUS_NSPS_NO_COVERAGE:
			return 0;

		case NET_REG_STATUS_HOME:
			return 1;

		case NET_REG_STATUS_NOSERV_SEARCHING:
			return 2;

		case NET_REG_STATUS_NOSERV_SIM_REJECTED_BY_NW:
			return 3;

		case NET_REG_STATUS_ROAM:
			return 5;

		default:
			return 4;
	}
}

void reg_status_ind_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct network_data *nd = opaque;
	struct network_status st;
	st.status = -1;
	st.lac = -1;
	st.cid = -1;
	st.technology = -1;

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		return;
	}

	/* package too small */
	if(len < 3)
		return;
	
	/* check if we received a status package */
	if(msg[0] != NET_REG_STATUS_IND && msg[0] != NET_REG_STATUS_GET_RESP)
		return;
	
	if(msg[0] == NET_REG_STATUS_GET_RESP && msg[1] != NET_CAUSE_OK) {
		g_warning("Request failed: %s", net_isi_cause_name(msg[1]));
		return;
	}

	if(decode_reg_status(nd, msg+3, len-3, &st)) {
		/* info message */
		g_message("Status: %s, LAC: 0x%X, CID: 0x%X, Technology: %d",
		          net_status_name(st.status), st.lac, st.cid, st.technology);

		/* call callback if set */
		if(nd->status_callback != NULL)
			nd->status_callback(&st, nd->user_data);
		else
			g_debug("no status callback defined!");
	}
}

bool reg_status_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	reg_status_ind_cb(client, data, len, object, opaque);
	return true;
}

gboolean isi_network_request_status(struct network_data *nd) {
	const unsigned char msg[] = {
		NET_REG_STATUS_GET_REQ
	};

	return g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_TIMEOUT, reg_status_resp_cb, nd) ? TRUE : FALSE;
}

gboolean isi_network_subscribe_status(struct network_data *nd) {
	return !g_isi_subscribe(nd->client, NET_REG_STATUS_IND, reg_status_ind_cb, nd);
}

void isi_network_unsubscribe_status(struct network_data *nd) {
	g_isi_unsubscribe(nd->client, NET_REG_STATUS_IND);
}

void network_rssi_ind_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct network_data *nd = opaque;

	if (!msg || len < 3 || msg[0] != NET_RSSI_IND)
		return;

	g_message("Strength: %d", msg[1]);

	if(nd->strength_callback != NULL)
		nd->strength_callback(msg[1], nd->user_data);
}

bool network_rssi_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct network_data *nd = opaque;

	GIsiSubBlockIter iter;
	int strength = -1;

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		return true;
	}

	if (len < 3 || msg[0] != NET_RSSI_GET_RESP)
		return false;

	if (msg[1] != NET_CAUSE_OK) {
		g_warning("Request failed: %s (0x%02X)",
			net_isi_cause_name(msg[1]), msg[1]);
		return true;
	}

	g_isi_sb_iter_init(&iter, msg, len, 3);

	while (g_isi_sb_iter_is_valid(&iter)) {
		switch (g_isi_sb_iter_get_id(&iter)) {
			case NET_RSSI_CURRENT: {
				guint8 rssi = 0;

				if (!g_isi_sb_iter_get_byte(&iter, &rssi, 2)) {
					g_debug("Could not get next byte!");
					return true;
				}

				strength = rssi != 0 ? rssi : -1;
				break;
			}

		default:
			g_debug("Skipping sub-block: %s (%zd bytes)",
				net_subblock_name(g_isi_sb_iter_get_id(&iter)),
				g_isi_sb_iter_get_len(&iter));
			break;
		}
		g_isi_sb_iter_next(&iter);
	}

	g_message("Strength: %d", strength);
	if(nd->strength_callback != NULL)
		nd->strength_callback(strength, nd->user_data);
	return true;
}

gboolean isi_network_request_strength(struct network_data *nd) {
	const unsigned char msg[] = {
		NET_RSSI_GET_REQ,
		NET_CS_GSM,
		NET_CURRENT_CELL_RSSI
	};

	return g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_TIMEOUT, network_rssi_resp_cb, nd) ? TRUE : FALSE;
}

gboolean isi_network_subscribe_strength(struct network_data *nd) {
	return !g_isi_subscribe(nd->client, NET_RSSI_IND, network_rssi_ind_cb, nd);
}

gboolean isi_network_unsubscribe_strength(struct network_data *nd) {
	g_isi_unsubscribe(nd->client, NET_RSSI_IND);
}

void network_reachable_cb(GIsiClient *client, bool alive, uint16_t object, void *opaque) {
	if (!alive) {
		g_critical("Unable to bootstrap network driver");
		return;
	}

	g_debug("%s (v%03d.%03d) reachable",
		pn_resource_name(g_isi_client_resource(client)),
		g_isi_version_major(client),
		g_isi_version_minor(client));

	/* subscribe to status changes */
	isi_network_subscribe_status(opaque);

	/* request current status */
	isi_network_request_status(opaque);
}

struct network_data* isi_network_create(GIsiModem *idx) {
	struct network_data *nd = g_try_new0(struct network_data, 1);

	if(!nd)
		return NULL;

	nd->client = g_isi_client_create(idx, PN_NETWORK);
	if(!nd->client) {
		g_free(nd);
		return NULL;
	}

	g_isi_verify(nd->client, network_reachable_cb, nd);

	return nd;
}

void isi_network_destroy(struct network_data *nd) {
	g_isi_client_destroy(nd->client);
	g_free(nd);
}


