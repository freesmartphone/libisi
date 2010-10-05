/* packet-isi.c
 * Dissector for ISI protocol
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
#include "isi-gps.h"

#define ISI_LTYPE 0xF5

int proto_isi = -1;

/* These are the handles of our subdissectors */
static dissector_handle_t data_handle=NULL;
static dissector_handle_t isi_handle;

/* Dissector table for the isi resource */
static dissector_table_t isi_resource_dissector_table;

/* Forward-declare the dissector functions */
static void dissect_isi(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree);

static const value_string hf_isi_device[] = {
	{0x00, "Modem" },
	{0x6c, "Host" },
	{0xFF, "Any" },
	{0x00, NULL },
};

static const value_string hf_isi_resource[] = {
	{0x01, "Call"},
	{0x02, "SMS"},
	{0x06, "Subscriber Services"},
	{0x08, "SIM Authentication"},
	{0x09, "SIM"},
	{0x0A, "Network"},
	{0x10, "Indication"},
	{0x15, "MTC"},
	{0x1B, "Phone Information"},
	{0x31, "GPRS"},
	{0x54, "GPS"},
	{0x62, "EPOC Info"}
};

static guint32 hf_isi_rdev = -1;
static guint32 hf_isi_sdev = -1;
static guint32 hf_isi_res  = -1;
static guint32 hf_isi_len  = -1;
static guint32 hf_isi_robj = -1;
static guint32 hf_isi_sobj = -1;
static guint32 hf_isi_id   = -1;

/* Subtree handles: set by register_subtree_array */
static guint32 ett_isi = -1;
guint32 ett_isi_msg = -1;

/* Handler registration */
void proto_reg_handoff_isi(void) {
	static gboolean initialized=FALSE;

	if (!initialized) {
		data_handle = find_dissector("data");
		isi_handle = create_dissector_handle(dissect_isi, proto_isi);
		dissector_add("sll.ltype", ISI_LTYPE, isi_handle);

		/* handoff resource dissectors */
		proto_reg_handoff_isi_sim_auth();
		proto_reg_handoff_isi_gps();
	}
}

void proto_register_isi(void) {
	/* A header field is something you can search/filter on.
	 * 
	 * We create a structure to register our fields. It consists of an
	 * array of hf_register_info structures, each of which are of the format
	 * {&(field id), {name, abbrev, type, display, strings, bitmask, blurb, HFILL}}.
	 */
	static hf_register_info hf[] = {
		{ &hf_isi_rdev,
		  { "Receiver Device", "isi.rdev", FT_UINT8, BASE_HEX,
		    VALS(hf_isi_device), 0x0, "Receiver Device ID", HFILL }},
		{ &hf_isi_sdev,
		  { "Sender Device", "isi.sdev", FT_UINT8, BASE_HEX,
		    VALS(hf_isi_device), 0x0, "Sender Device ID", HFILL }},
		{ &hf_isi_res,
		  { "Resource", "isi.res", FT_UINT8, BASE_HEX,
		    VALS(hf_isi_resource), 0x0, "Resource ID", HFILL }},
		{ &hf_isi_len,
		  { "Length", "isi.len", FT_UINT16, BASE_DEC,
		    NULL, 0x0, "Length", HFILL }},
		{ &hf_isi_robj,
		  { "Receiver Object", "isi.robj", FT_UINT8, BASE_HEX,
		    NULL, 0x0, "Receiver Object", HFILL }},
		{ &hf_isi_sobj,
		  { "Sender Object", "isi.sobj", FT_UINT8, BASE_HEX,
		    NULL, 0x0, "Sender Object", HFILL }},
		{ &hf_isi_id,
		  { "Packet ID", "isi.id", FT_UINT8, BASE_DEC,
		    NULL, 0x0, "Packet ID", HFILL }}
    };

	static gint *ett[] = {
		&ett_isi,
		&ett_isi_msg
	};

 	proto_isi = proto_register_protocol("Intelligent Service Interface", "ISI", "isi");

	proto_register_field_array(proto_isi, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
	register_dissector("isi", dissect_isi, proto_isi);

	/* create new dissector table for isi resource */
	isi_resource_dissector_table = register_dissector_table("isi.resource", "ISI resource", FT_UINT8, BASE_HEX);

	/* register resource dissectors */
	proto_register_isi_sim_auth();
	proto_register_isi_gps();
}

/* The dissector itself */
static void dissect_isi(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree) {
	proto_tree *isi_tree = NULL;
	guint position = 0;
	proto_item *item = NULL;
	tvbuff_t *content = NULL;

	guint8 src = 0;
	guint8 dst = 0;
	guint8 resource = 0;
	guint16 length = 0;

	if(check_col(pinfo->cinfo, COL_PROTOCOL)) 
		col_set_str(pinfo->cinfo, COL_PROTOCOL, "ISI");
	
	if(check_col(pinfo->cinfo,COL_INFO))
		col_clear(pinfo->cinfo,COL_INFO);

	if(tree) {
		/* If tree != NULL, we're doing a detailed dissection of the
		 * packet, so we need to construct a tree. */

		/* Start with a top-level item to add everything else to */
		item = proto_tree_add_item(tree, proto_isi, tvb, position, -1, FALSE);
		isi_tree = proto_item_add_subtree(item, ett_isi);

		/* Common Phonet/ISI Header */
		proto_tree_add_item(isi_tree, hf_isi_rdev, tvb, 0, 1, FALSE);
		proto_tree_add_item(isi_tree, hf_isi_sdev, tvb, 1, 1, FALSE);
		proto_tree_add_item(isi_tree, hf_isi_res,  tvb, 2, 1, FALSE);
		proto_tree_add_item(isi_tree, hf_isi_len,  tvb, 3, 2, FALSE);
		proto_tree_add_item(isi_tree, hf_isi_robj, tvb, 5, 1, FALSE);
		proto_tree_add_item(isi_tree, hf_isi_sobj, tvb, 6, 1, FALSE);
		proto_tree_add_item(isi_tree, hf_isi_id,   tvb, 7, 1, FALSE);

		length = tvb_get_ntohs(tvb, 3) - 3;
		resource = tvb_get_guint8(tvb, 2);
		dst = tvb_get_guint8(tvb, 0);
		src = tvb_get_guint8(tvb, 1);

		if(tvb->length - 8 < length) {
			expert_add_info_format(pinfo, item, PI_PROTOCOL, PI_WARN, "Broken Length (%d > %d)", length, tvb->length-8);
			length = tvb->length - 8;
		}

		col_set_str(pinfo->cinfo, COL_DEF_SRC, val_to_str_const(src, hf_isi_device, "Unknown"));
		col_set_str(pinfo->cinfo, COL_DEF_DST, val_to_str_const(dst, hf_isi_device, "Unknown"));

		content = tvb_new_subset(tvb, 8, length, length);

		/* Call subdissector depending on the resource ID */
		if(!dissector_try_port(isi_resource_dissector_table, resource, content, pinfo, isi_tree))
			call_dissector(data_handle, content, pinfo, isi_tree);
	}
}
