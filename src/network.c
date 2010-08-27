/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <errno.h>
#include <string.h>

#include "opcodes/network.h"
#include "network.h"
#include "modem.h"
#include "debug.h"
#include "gisi/iter.h"
#include "helper.h"

gboolean decode_reg_status(struct isi_network *nd, const guint8 *msg, size_t len, struct network_status *st) {
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
	struct isi_cb_data *cbd = opaque;
	struct isi_network *nd = cbd->subsystem;
	isi_network_status_cb cb = cbd->callback;
	void *user_data = cbd->data;
	struct network_status st;
	st.status = -1;
	st.lac = -1;
	st.cid = -1;
	st.technology = -1;

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	if(!cb) {
		g_warning("no callback defined!");
		return;
	}

	/* package too small */
	if(len < 3)
		goto error;
	
	/* check if we received a status package */
	if(msg[0] != NET_REG_STATUS_IND && msg[0] != NET_REG_STATUS_GET_RESP)
		goto error;
	
	if(msg[0] == NET_REG_STATUS_GET_RESP && msg[1] != NET_CAUSE_OK) {
		g_warning("Request failed: %s", net_isi_cause_name(msg[1]));
		goto error;
	}

	if(decode_reg_status(nd, msg+3, len-3, &st)) {
		/* info message */
		g_message("Status: %s, LAC: 0x%X, CID: 0x%X, Technology: %d",
		          net_status_name(st.status), st.lac, st.cid, st.technology);
		cb(FALSE, &st, user_data);
		return;
	}

	error:
		cb(TRUE, NULL, user_data);
}

gboolean reg_status_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	reg_status_ind_cb(client, data, len, object, opaque);
	isi_cb_data_free(opaque);
	return TRUE;
}

void isi_network_request_status(struct isi_network *nd, isi_network_status_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);

	const unsigned char msg[] = {
		NET_REG_STATUS_GET_REQ
	};

	if(!cbd || !g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_TIMEOUT, reg_status_resp_cb, nd)) {
		isi_cb_data_free(cbd);
		cb(TRUE, NULL, user_data);
	}
}

void isi_network_subscribe_status(struct isi_network *nd, isi_network_status_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	if(!cbd || g_isi_subscribe(nd->client, NET_REG_STATUS_IND, reg_status_ind_cb, nd)) {
		isi_cb_data_free(cbd);
		cb(TRUE, 0, user_data);
	}
}

void isi_network_unsubscribe_status(struct isi_network *nd) {
	g_isi_unsubscribe(nd->client, NET_REG_STATUS_IND);
}

void network_rssi_ind_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = opaque;
	struct isi_network *nd = cbd->subsystem;
	isi_network_strength_cb cb = cbd->callback;
	void *user_data = cbd->data;

	if(!msg || len < 3 || msg[0] != NET_RSSI_IND) {
		cb(TRUE, 0, user_data);
		return;
	}

	g_message("Strength: %d", msg[1]);

	cb(FALSE, msg[1], user_data);
}

gboolean network_rssi_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *opaque) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = opaque;
	struct isi_network *nd = cbd->subsystem;
	isi_network_strength_cb cb = cbd->callback;
	void *user_data = cbd->data;
	isi_cb_data_free(cbd);

	GIsiSubBlockIter iter;
	int strength = -1;

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		cb(TRUE, 0, user_data);
		return TRUE;
	}

	if (len < 3 || msg[0] != NET_RSSI_GET_RESP) {
		cb(TRUE, 0, user_data);
		return FALSE;
	}

	if (msg[1] != NET_CAUSE_OK) {
		g_warning("Request failed: %s (0x%02X)",
			net_isi_cause_name(msg[1]), msg[1]);
		cb(TRUE, 0, user_data);
		return TRUE;
	}

	g_isi_sb_iter_init(&iter, msg, len, 3);

	while (g_isi_sb_iter_is_valid(&iter)) {
		switch (g_isi_sb_iter_get_id(&iter)) {
			case NET_RSSI_CURRENT: {
				guint8 rssi = 0;

				if (!g_isi_sb_iter_get_byte(&iter, &rssi, 2)) {
					g_debug("Could not get next byte!");
					cb(TRUE, 0, user_data);
					return TRUE;
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
	cb(FALSE, strength, user_data);
	return TRUE;
}

void isi_network_request_strength(struct isi_network *nd, isi_network_strength_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);

	const unsigned char msg[] = {
		NET_RSSI_GET_REQ,
		NET_CS_GSM,
		NET_CURRENT_CELL_RSSI
	};

	if(cbd && g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_TIMEOUT, network_rssi_resp_cb, cbd))
		return

	cb(TRUE, 0, user_data);
	isi_cb_data_free(cbd);
}

void isi_network_subscribe_strength(struct isi_network *nd, isi_network_strength_cb cb, void *user_data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, user_data);
	if(!cbd || g_isi_subscribe(nd->client, NET_RSSI_IND, network_rssi_ind_cb, nd)) {
		isi_cb_data_free(cbd);
		cb(TRUE, 0, user_data);
	}
}

void isi_network_unsubscribe_strength(struct isi_network *nd) {
	g_isi_unsubscribe(nd->client, NET_RSSI_IND);
}

void network_reachable_cb(GIsiClient *client, gboolean alive, uint16_t object, void *user_data) {
	struct isi_cb_data *cbd = user_data;
	isi_subsystem_reachable_cb cb = cbd->callback;

	if (!alive) {
		g_critical("Unable to bootstrap network driver");
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

struct isi_network* isi_network_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data) {
	struct isi_network *nd = calloc(sizeof(struct isi_network), 1);
	struct isi_cb_data *cbd = isi_cb_data_new(NULL, cb, data);

	if(!nd || !cbd || !modem->idx)
		goto error;

	nd->client = g_isi_client_create(modem->idx, PN_NETWORK);
	if(!nd->client)
		goto error;

	g_isi_verify(nd->client, network_reachable_cb, cbd);

	return nd;

	error:
		cb(TRUE, data);
		if(nd)
			free(nd);
		isi_cb_data_free(cbd);
		return NULL;
}

void isi_network_destroy(struct isi_network *nd) {
	if(!nd)
		return;
	g_isi_client_destroy(nd->client);
	free(nd);
}

gboolean set_manual_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	struct isi_network *nd = cbd->subsystem;
	isi_network_register_cb cb = cbd->callback;

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	if(len < 3 || msg[0] != NET_SET_RESP)
		goto error;

	if(msg[1] != NET_CAUSE_OK) {
		g_warning("Request failed: %s", net_isi_cause_name(msg[1]));
		goto error;
	}

	cb(FALSE, cbd->data);
	nd->last_reg_mode = NET_SELECT_MODE_MANUAL;
	goto out;

error:
	cb(TRUE, cbd->data);

out:
	isi_cb_data_free(cbd);
	return TRUE;
}

gboolean set_auto_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	struct isi_network *nd = cbd->subsystem;
	isi_network_register_cb cb = cbd->callback;

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	if(!msg || len < 3 || msg[0] != NET_SET_RESP)
		goto error;

	if(msg[1] != NET_CAUSE_OK) {
		g_debug("Request failed: %s", net_isi_cause_name(msg[1]));
		goto error;
	}

	cb(FALSE, cbd->data);
	nd->last_reg_mode = NET_SELECT_MODE_AUTOMATIC;
	goto out;

error:
	cb(FALSE, cbd->data);

out:
	isi_cb_data_free(cbd);
	return TRUE;
}

void isi_network_register_manual(struct isi_network *nd, const char *mcc, const char *mnc, isi_network_register_cb cb, void *data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, data);

	/* generate bcd */
	char bcd[3];
	bcd[0] = (mcc[0] - '0') | (mcc[1] - '0') << 4;
	bcd[1] = (mcc[2] - '0');
	bcd[1] |= (mnc[2] == '\0' ? 0x0f : (mnc[2] - '0')) << 4;
	bcd[2] = (mnc[0] - '0') | (mnc[1] - '0') << 4;

	const unsigned char msg[] = {
		NET_SET_REQ,
		0x00,  /* Registered in another protocol? */
		0x02,  /* Sub-block count */
		NET_OPERATOR_INFO_COMMON,
		0x04,  /* Sub-block length */
		NET_SELECT_MODE_MANUAL,
		0x00,  /* Index not used */
		NET_GSM_OPERATOR_INFO,
		0x08,  /* Sub-block length */
		bcd[0], bcd[1], bcd[2],
		NET_GSM_BAND_INFO_NOT_AVAIL,  /* Pick any supported band */
		0x00, 0x00  /* Filler */
	};

	g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_SET_TIMEOUT, set_manual_resp_cb, cbd);
}

void isi_network_register_auto(struct isi_network *nd, isi_network_register_cb cb, void *data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, data);

	const unsigned char msg[] = {
		NET_SET_REQ,
		0x00,  /* Registered in another protocol? */
		0x01,  /* Sub-block count */
		NET_OPERATOR_INFO_COMMON,
		0x04,  /* Sub-block length */
		nd->last_reg_mode == NET_SELECT_MODE_AUTOMATIC
			? NET_SELECT_MODE_USER_RESELECTION
			: NET_SELECT_MODE_AUTOMATIC,
		0x00  /* Index not used */
	};

	if(cbd && g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_SET_TIMEOUT, set_auto_resp_cb, cbd))
		return;

	cb(TRUE, data);
	if(cbd)
		isi_cb_data_free(cbd);
}

void isi_network_deregister(struct isi_network *nd, isi_network_register_cb cb, void *data) {
	g_critical("Network deregister is not implemented!");
	cb(TRUE, data);
}

gboolean name_get_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	isi_network_operator_cb cb = cbd->callback;

	struct network_operator op;
	GIsiSubBlockIter iter;

	memset(&op, 0, sizeof(struct network_operator)); // FIXME: check this struct

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	if(len < 3 || msg[0] != NET_OPER_NAME_READ_RESP)
		return FALSE;

	if(msg[1] != NET_CAUSE_OK) {
		g_warning("Request failed: %s", net_isi_cause_name(msg[1]));
		goto error;
	}

	g_isi_sb_iter_init(&iter, msg, len, 7);

	while(g_isi_sb_iter_is_valid(&iter)) {
		switch(g_isi_sb_iter_get_id(&iter)) {
			case NET_GSM_OPERATOR_INFO:
				if(!g_isi_sb_iter_get_oper_code(&iter, op.mcc, op.mnc, 2))
					goto error;
				break;
			case NET_OPER_NAME_INFO: {
				char *tag = NULL;
				guint8 taglen = 0;

				if(!g_isi_sb_iter_get_byte(&iter, &taglen, 3)
					|| !g_isi_sb_iter_get_alpha_tag(&iter, &tag, taglen * 2, 4))
					goto error;

				strncpy(op.name, tag, ISI_MAX_OPERATOR_NAME_LENGTH);
				op.name[ISI_MAX_OPERATOR_NAME_LENGTH] = '\0';
				g_free(tag);
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

	cb(FALSE, &op, cbd->data);
	goto out;

error:
	cb(TRUE, NULL, cbd->data);

out:
	isi_cb_data_free(cbd);
	return TRUE;
}

void isi_network_current_operator(struct isi_network *nd, isi_network_operator_cb cb, void *data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, data);

	const unsigned char msg[] = {
		NET_OPER_NAME_READ_REQ,
		NET_HARDCODED_LATIN_OPER_NAME,
		ISI_MAX_OPERATOR_NAME_LENGTH,
		0x00, 0x00,  /* Index not used */
		0x00,  /* Filler */
		0x00  /* No sub-blocks */
	};

	if(cbd && g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_TIMEOUT, name_get_resp_cb, cbd))
		return;

	cb(TRUE, NULL, data);
	isi_cb_data_free(cbd);
}

gboolean available_resp_cb(GIsiClient *client, const void *restrict data, size_t len, uint16_t object, void *user_data) {
	const unsigned char *msg = data;
	struct isi_cb_data *cbd = user_data;
	isi_network_operator_list_cb cb = cbd->callback;
	struct network_operator *list = NULL;
	int total = 0;

	GIsiSubBlockIter iter;
	int common = 0;
	int detail = 0;

	if(!msg) {
		g_warning("ISI client error: %d", g_isi_client_error(client));
		goto error;
	}

	if(len < 3 || msg[0] != NET_AVAILABLE_GET_RESP)
		return FALSE;

	if(msg[1] != NET_CAUSE_OK) {
		g_warning("Request failed: %s", net_isi_cause_name(msg[1]));
		goto error;
	}

	/* Each description of an operator has a pair of sub-blocks */
	total = msg[2] / 2;
	list = alloca(total * sizeof(struct network_operator));

	g_isi_sb_iter_init(&iter, msg, len, 3);

	while(g_isi_sb_iter_is_valid(&iter)) {
		switch(g_isi_sb_iter_get_id(&iter)) {
			case NET_AVAIL_NETWORK_INFO_COMMON: {
				struct network_operator *op;
				char *tag = NULL;
				guint8 taglen = 0;
				guint8 status = 0;

				if (!g_isi_sb_iter_get_byte(&iter, &status, 2))
					goto error;

				if (!g_isi_sb_iter_get_byte(&iter, &taglen, 5))
					goto error;

				if (!g_isi_sb_iter_get_alpha_tag(&iter, &tag, taglen * 2, 6))
					goto error;

				op = list + common++;
				op->status = status;

				strncpy(op->name, tag, ISI_MAX_OPERATOR_NAME_LENGTH);
				op->name[ISI_MAX_OPERATOR_NAME_LENGTH] = '\0';
				g_free(tag);
				break;
			}
			case NET_DETAILED_NETWORK_INFO: {
				struct network_operator *op;

				op = list + detail++;
				if (!g_isi_sb_iter_get_oper_code(&iter, op->mcc, op->mnc, 2))
					goto error;
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

	if (common == detail && detail == total) {
		cb(FALSE, list, total, cbd->data);
		goto out;
	}

error:
	cb(TRUE, NULL, 0, cbd->data);

out:
	isi_cb_data_free(cbd);
	return TRUE;
}

void isi_network_list_operators(struct isi_network *nd, isi_network_operator_list_cb cb, void *data) {
	struct isi_cb_data *cbd = isi_cb_data_new(nd, cb, data);

	const unsigned char msg[] = {
		NET_AVAILABLE_GET_REQ,
		NET_MANUAL_SEARCH,
		0x01,  /* Sub-block count */
		NET_GSM_BAND_INFO,
		0x04,  /* Sub-block length */
		NET_GSM_BAND_ALL_SUPPORTED_BANDS,
		0x00
	};

	if(cbd && g_isi_request_make(nd->client, msg, sizeof(msg), NETWORK_SCAN_TIMEOUT, available_resp_cb, cbd))
		return;

error:
	cb(TRUE, NULL, 0, data);
	isi_cb_data_free(cbd);
}
