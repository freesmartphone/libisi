/* isi-simauth.c
 * Dissector for ISI's SIM authentication resource
 * Copyright 2010, Sebastian Reichel <sre@ring0.de>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <epan/prefs.h>
#include <epan/packet.h>

#include "packet-isi.h"
#include "isi-simauth.h"

static const value_string isi_sim_auth_id[] = {
	{0x01, "SIM_AUTH_PROTECTED_REQ"},
	{0x02, "SIM_AUTH_PROTECTED_RESP"},
	{0x04, "SIM_AUTH_UPDATE_REQ"},
	{0x05, "SIM_AUTH_UPDATE_SUCCESS_RESP"},
	{0x06, "SIM_AUTH_UPDATE_FAIL_RESP"},
	{0x07, "SIM_AUTH_REQ"},
	{0x08, "SIM_AUTH_SUCCESS_RESP"},
	{0x09, "SIM_AUTH_FAIL_RESP"},
	{0x10, "SIM_AUTH_STATUS_IND"},
	{0x11, "SIM_AUTH_STATUS_REQ"},
	{0x12, "SIM_AUTH_STATUS_RESP"},
	{0x00, NULL }
};

static const value_string isi_sim_auth_pw_type[] = {
	{0x02, "SIM_AUTH_PIN"},
	{0x03, "SIM_AUTH_PUK"},
	{0x63, "SIM_AUTH_NONE"},
	{0x00, NULL}
};

static const value_string isi_sim_auth_protection_req[] = {
	{0x00, "SIM_AUTH_PROTECTION_DISABLE"},
	{0x01, "SIM_AUTH_PROTECTION_ENABLE"},
	{0x04, "SIM_AUTH_PROTECTION_STATUS"},
	{0x00, NULL}
};

static const value_string isi_sim_auth_resp[] = {
	{0x02, "SIM_AUTH_STATUS_RESP_NEED_PIN"},
	{0x03, "SIM_AUTH_STATUS_RESP_NEED_PUK"},
	{0x05, "SIM_AUTH_STATUS_RESP_RUNNING"},
	{0x07, "SIM_AUTH_STATUS_RESP_INIT"},
	{0x00, NULL}
};

static const value_string isi_sim_auth_indication[] = {
	{0x01, "SIM_AUTH_NEED_AUTH"},
	{0x02, "SIM_AUTH_NEED_NO_AUTH"},
	{0x03, "SIM_AUTH_VALID"},
	{0x04, "SIM_AUTH_INVALID"},
	{0x05, "SIM_AUTH_AUTHORIZED"},
	{0x06, "SIM_AUTH_IND_CONFIG"},
	{0x00, NULL}
};

static const value_string isi_sim_auth_indication_cfg[] = {
	{0x0B, "SIM_AUTH_PIN_PROTECTED_DISABLE"},
	{0x0C, "SIM_AUTH_PIN_PROTECTED_ENABLE"},
	{0x00, NULL}
};

static dissector_handle_t isi_sim_auth_handle;
static void dissect_isi_sim_auth(tvbuff_t *tvb, packet_info *pinfo, proto_item *tree);

static guint32 hf_isi_sim_auth_cmd = -1;
static guint32 hf_isi_sim_auth_status_rsp = -1;
static guint32 hf_isi_sim_auth_protection_req = -1;
static guint32 hf_isi_sim_auth_protection_rsp = -1;
static guint32 hf_isi_sim_auth_pin = -1;
static guint32 hf_isi_sim_auth_puk = -1;
static guint32 hf_isi_sim_auth_new_pin = -1;
static guint32 hf_isi_sim_auth_pw_type = -1;
static guint32 hf_isi_sim_auth_indication = -1;
static guint32 hf_isi_sim_auth_indication_cfg = -1;

void proto_reg_handoff_isi_sim_auth(void) {
	static gboolean initialized=FALSE;

	if (!initialized) {
		isi_sim_auth_handle = create_dissector_handle(dissect_isi_sim_auth, proto_isi);
		dissector_add("isi.resource", 0x08, isi_sim_auth_handle);
	}
}

void proto_register_isi_sim_auth(void) {
	static hf_register_info hf[] = {
		{ &hf_isi_sim_auth_cmd,
		  { "Command", "isi.sim.auth.cmd", FT_UINT8, BASE_HEX, isi_sim_auth_id, 0x0, "Command", HFILL }},
		{ &hf_isi_sim_auth_pw_type,
		  { "Password Type", "isi.sim.auth.type", FT_UINT8, BASE_HEX, isi_sim_auth_pw_type, 0x0, "Password Type", HFILL }},
		{ &hf_isi_sim_auth_pin,
		  { "PIN", "isi.sim.auth.pin", FT_STRING, BASE_NONE, NULL, 0x0, "PIN", HFILL }},
		{ &hf_isi_sim_auth_puk,
		  { "PUK", "isi.sim.auth.puk", FT_STRING, BASE_NONE, NULL, 0x0, "PUK", HFILL }},
		{ &hf_isi_sim_auth_new_pin,
		  { "New PIN", "isi.sim.auth.new_pin", FT_STRING, BASE_NONE, NULL, 0x0, "New PIN", HFILL }},
		{ &hf_isi_sim_auth_protection_req,
		  { "Protection Request", "isi.sim.auth.request.protection", FT_UINT8, BASE_HEX, isi_sim_auth_protection_req, 0x0, "Protection Request", HFILL }},
		{ &hf_isi_sim_auth_protection_rsp,
		  { "Protection Response", "isi.sim.auth.response.protection", FT_BOOLEAN, BASE_HEX, NULL, 0x0, "Protection Response", HFILL }},
		{ &hf_isi_sim_auth_status_rsp,
		  { "Status Response", "isi.sim.auth.response.status", FT_UINT8, BASE_HEX, isi_sim_auth_resp, 0x0, "Status Response", HFILL }},
		{ &hf_isi_sim_auth_indication,
		  { "Indication", "isi.sim.auth.indication", FT_UINT8, BASE_HEX, isi_sim_auth_indication, 0x0, "Indication", HFILL }},
		{ &hf_isi_sim_auth_indication_cfg,
		  { "Configuration", "isi.sim.auth.cfg", FT_UINT8, BASE_HEX, isi_sim_auth_indication_cfg, 0x0, "Configuration", HFILL }}
	};

	proto_register_field_array(proto_isi, hf, array_length(hf));
	register_dissector("isi.sim.auth", dissect_isi_sim_auth, proto_isi);
}

static void dissect_isi_sim_auth(tvbuff_t *tvb, packet_info *pinfo, proto_item *isitree) {
	proto_item *item = NULL;
	proto_tree *tree = NULL;
	guint8 cmd, code;

	if(isitree) {
		item = proto_tree_add_text(isitree, tvb, 0, -1, "Payload");
		tree = proto_item_add_subtree(item, ett_isi_msg);

		proto_tree_add_item(tree, hf_isi_sim_auth_cmd, tvb, 0, 1, FALSE);
		cmd = tvb_get_guint8(tvb, 0);

		switch(cmd) {
			case 0x01: // SIM_AUTH_PROTECTED_REQ
				proto_tree_add_item(tree, hf_isi_sim_auth_protection_req, tvb, 2, 1, FALSE);
				cmd = tvb_get_guint8(tvb, 2);
				switch(cmd) {
					case 0x00: // DISABLE
						proto_tree_add_item(tree, hf_isi_sim_auth_pin, tvb, 3, -1, FALSE);
						col_set_str(pinfo->cinfo, COL_INFO, "disable SIM startup protection");
						break;
					case 0x01: // ENABLE
						proto_tree_add_item(tree, hf_isi_sim_auth_pin, tvb, 3, -1, FALSE);
						col_set_str(pinfo->cinfo, COL_INFO, "enable SIM startup protection");
						break;
					case 0x04: // STATUS
						col_set_str(pinfo->cinfo, COL_INFO, "get SIM startup protection status");
						break;
					default:
						col_set_str(pinfo->cinfo, COL_INFO, "unknown SIM startup protection package");
						break;
				}
				break;
			case 0x02: // SIM_AUTH_PROTECTED_RESP
				proto_tree_add_item(tree, hf_isi_sim_auth_protection_rsp, tvb, 1, 1, FALSE);
				if(tvb_get_guint8(tvb, 1))
					col_set_str(pinfo->cinfo, COL_INFO, "SIM startup protection enabled");
				else
					col_set_str(pinfo->cinfo, COL_INFO, "SIM startup protection disabled");
				break;
			case 0x04: // SIM_AUTH_UPDATE_REQ
				proto_tree_add_item(tree, hf_isi_sim_auth_pw_type, tvb, 1, 1, FALSE);
				code = tvb_get_guint8(tvb, 1);
				switch(code) {
					case 0x02: // PIN
						col_set_str(pinfo->cinfo, COL_INFO, "update SIM PIN");
						proto_tree_add_item(tree, hf_isi_sim_auth_pin, tvb, 2, 11, FALSE);
						proto_tree_add_item(tree, hf_isi_sim_auth_new_pin, tvb, 13, 11, FALSE);
						break;
					case 0x03: // PUK
						col_set_str(pinfo->cinfo, COL_INFO, "update SIM PUK");
						break;
					default:
						col_set_str(pinfo->cinfo, COL_INFO, "unknown SIM Authentication update request");
						break;
				}
				break;
			case 0x05: // SIM_AUTH_UPDATE_SUCCESS_RESP
				col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication update successful");
				break;
			case 0x06: // SIM_AUTH_UPDATE_FAIL_RESP
				col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication update failed");
				break;
			case 0x07: // SIM_AUTH_REQ
				proto_tree_add_item(tree, hf_isi_sim_auth_pw_type, tvb, 1, 1, FALSE);
				code = tvb_get_guint8(tvb, 1);
				switch(code) {
					case 0x02: // PIN
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication with PIN");
						proto_tree_add_item(tree, hf_isi_sim_auth_pin, tvb, 2, 11, FALSE);
						break;
					case 0x03: // PUK
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication with PUK");
						proto_tree_add_item(tree, hf_isi_sim_auth_puk, tvb, 2, 11, FALSE);
						proto_tree_add_item(tree, hf_isi_sim_auth_new_pin, tvb, 13, 11, FALSE);
						break;
					default:
						col_set_str(pinfo->cinfo, COL_INFO, "unknown SIM Authentication request");
						break;
				}
				break;
			case 0x08: // SIM_AUTH_SUCCESS_RESP
				col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication successful");
				break;
			case 0x09: // SIM_AUTH_FAIL_RESP
				col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication failed");
				break;
			case 0x10: // SIM_AUTH_STATUS_IND
				proto_tree_add_item(tree, hf_isi_sim_auth_indication, tvb, 1, 1, FALSE);
				code = tvb_get_guint8(tvb, 1);
				proto_tree_add_item(tree, hf_isi_sim_auth_pw_type, tvb, 2, 1, FALSE);
				switch(code) {
					case 0x01:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication indication: Authentication needed");
						break;
					case 0x02:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication indication: No Authentication needed");
						break;
					case 0x03:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication indication: Authentication valid");
						break;
					case 0x04:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication indication: Authentication invalid");
						break;
					case 0x05:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication indication: Authorized");
						break;
					case 0x06:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication indication: Config");
						proto_tree_add_item(tree, hf_isi_sim_auth_indication_cfg, tvb, 3, 1, FALSE);
						break;
					default:
						col_set_str(pinfo->cinfo, COL_INFO, "unknown SIM Authentication indication");
						break;
				}
				break;
			case 0x11: // SIM_AUTH_STATUS_REQ
				col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication status request");
				break;
			case 0x12: // SIM_AUTH_STATUS_RESP
				proto_tree_add_item(tree, hf_isi_sim_auth_status_rsp, tvb, 1, 1, FALSE);
				code = tvb_get_guint8(tvb, 1);
				switch(code) {
					case 0x02:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication status: need PIN");
						break;
					case 0x03:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication status: need PUK");
						break;
					case 0x05:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication status: running");
						break;
					case 0x07:
						col_set_str(pinfo->cinfo, COL_INFO, "SIM Authentication status: initializing");
						break;
					default:
						col_set_str(pinfo->cinfo, COL_INFO, "unknown SIM Authentication status response package");
						break;
				}
				break;
			default:
				col_set_str(pinfo->cinfo, COL_INFO, "unknown SIM Authentication package");
				break;
		}
	}
}
