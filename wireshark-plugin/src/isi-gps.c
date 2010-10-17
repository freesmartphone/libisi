/* isi-gps.c
 * Dissector for ISI's GPS resource
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
 *
 * GPS Data Packet Reverse Engineered by Luke Dashjr <luke@dashjr.org>
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <epan/prefs.h>
#include <epan/packet.h>

#include "packet-isi.h"
#include "isi-gps.h"

/* centimeter per second to kilometer per hour */
#define CMS_TO_KMH 0.036
#define SAT_PKG_LEN 12

static const value_string isi_gps_id[] = {
	//{0x0d, "GPS_UNKNOWN_0D"},
	//{0x0e, "GPS_UNKNOWN_0E"},
	{0x7d, "GPS_STATUS_IND"},
	//{0x7e, "GPS_UNKNOWN_7E"},
	//{0x7f, "GPS_UNKNOWN_7F"},
	//{0x82, "GPS_UNKNOWN_82"},
	//{0x83, "GPS_UNKNOWN_83"},
	//{0x84, "GPS_UNKNOWN_84"},
	//{0x85, "GPS_UNKNOWN_85"},
	//{0x90, "GPS_UNKNOWN_90"},
	//{0x91, "GPS_UNKNOWN_91"},
	{0x90, "GPS_POWER_STATUS_REQ"},
	{0x91, "GPS_POWER_STATUS_RSP"},
	{0x92, "GPS_DATA_IND"},
	{0x00, NULL }
};

static const value_string isi_gps_sub_id[] = {
	{0x02, "GPS_POSITION"},
	{0x03, "GPS_TIME_DATE"},
	{0x04, "GPS_MOVEMENT"},
	{0x05, "GPS_SAT_INFO"},
	{0x07, "GPS_CELL_INFO_GSM"},
	{0x08, "GPS_CELL_INFO_WCDMA"},
	{0x00, NULL }
};

static const value_string isi_gps_status[] = {
	{0x00, "GPS_DISABLED"},
	{0x01, "GPS_NO_LOCK"},
	{0x02, "GPS_LOCK"},
	{0x00, NULL }
};

static dissector_handle_t isi_gps_handle;
static void dissect_isi_gps(tvbuff_t *tvb, packet_info *pinfo, proto_item *tree);

static guint32 hf_isi_gps_cmd = -1;
static guint32 hf_isi_gps_sub_pkgs = -1;
static guint32 hf_isi_gps_sub_type = -1;
static guint32 hf_isi_gps_sub_len = -1;
static guint32 hf_isi_gps_status = -1;
static guint32 hf_isi_gps_year = -1;
static guint32 hf_isi_gps_month = -1;
static guint32 hf_isi_gps_day = -1;
static guint32 hf_isi_gps_hour = -1;
static guint32 hf_isi_gps_minute = -1;
static guint32 hf_isi_gps_second = -1;
static guint32 hf_isi_gps_latitude = -1;
static guint32 hf_isi_gps_longitude = -1;
static guint32 hf_isi_gps_eph = -1;
static guint32 hf_isi_gps_altitude = -1;
static guint32 hf_isi_gps_epv = -1;
static guint32 hf_isi_gps_course = -1;
static guint32 hf_isi_gps_epd = -1;
static guint32 hf_isi_gps_speed = -1;
static guint32 hf_isi_gps_eps = -1;
static guint32 hf_isi_gps_climb = -1;
static guint32 hf_isi_gps_epc = -1;
static guint32 hf_isi_gps_mcc = -1;
static guint32 hf_isi_gps_mnc = -1;
static guint32 hf_isi_gps_lac = -1;
static guint32 hf_isi_gps_cid = -1;
static guint32 hf_isi_gps_ucid = -1;
static guint32 hf_isi_gps_satellites = -1;
static guint32 hf_isi_gps_prn = -1;
static guint32 hf_isi_gps_sat_used = -1;
static guint32 hf_isi_gps_sat_strength = -1;
static guint32 hf_isi_gps_sat_elevation = -1;
static guint32 hf_isi_gps_sat_azimuth = -1;

void proto_reg_handoff_isi_gps(void) {
	static gboolean initialized=FALSE;

	if (!initialized) {
		isi_gps_handle = create_dissector_handle(dissect_isi_gps, proto_isi);
		dissector_add("isi.resource", 0x54, isi_gps_handle);
	}
}

void proto_register_isi_gps(void) {
	static hf_register_info hf[] = {
		{ &hf_isi_gps_cmd,
		  { "Command", "isi.gps.cmd", FT_UINT8, BASE_HEX, isi_gps_id, 0x0, "Command", HFILL }},
		{ &hf_isi_gps_sub_pkgs,
		  { "Number of Subpackets", "isi.gps.pkgs", FT_UINT8, BASE_DEC, NULL, 0x0, "Number of Subpackets", HFILL }},
		{ &hf_isi_gps_sub_type,
		  { "Subpacket Type", "isi.gps.sub.type", FT_UINT8, BASE_HEX, isi_gps_sub_id, 0x0, "Subpacket Type", HFILL }},
		{ &hf_isi_gps_sub_len,
		  { "Subpacket Length", "isi.gps.sub.len", FT_UINT8, BASE_DEC, NULL, 0x0, "Subpacket Length", HFILL }},
		{ &hf_isi_gps_status,
		  { "Status", "isi.gps.status", FT_UINT8, BASE_HEX, isi_gps_status, 0x0, "Status", HFILL }},
		{ &hf_isi_gps_year,
		  { "Year", "isi.gps.date.year", FT_UINT16, BASE_DEC, NULL, 0x0, "Year", HFILL }},
		{ &hf_isi_gps_month,
		  { "Month", "isi.gps.date.month", FT_UINT8, BASE_DEC, NULL, 0x0, "Month", HFILL }},
		{ &hf_isi_gps_day,
		  { "Day", "isi.gps.date.day", FT_UINT8, BASE_DEC, NULL, 0x0, "Day", HFILL }},
		{ &hf_isi_gps_hour,
		  { "Hour", "isi.gps.time.hour", FT_UINT8, BASE_DEC, NULL, 0x0, "Hour", HFILL }},
		{ &hf_isi_gps_minute,
		  { "Minute", "isi.gps.time.minute", FT_UINT8, BASE_DEC, NULL, 0x0, "Minute", HFILL }},
		{ &hf_isi_gps_second,
		  { "Second", "isi.gps.time.second", FT_FLOAT, BASE_NONE, NULL, 0x0, "Second", HFILL }},
		{ &hf_isi_gps_latitude,
		  { "Latitude", "isi.gps.lat", FT_DOUBLE, BASE_NONE, NULL, 0x0, "Latitude", HFILL }},
		{ &hf_isi_gps_longitude,
		  { "Longitude", "isi.gps.lon", FT_DOUBLE, BASE_NONE, NULL, 0x0, "Longitude", HFILL }},
		{ &hf_isi_gps_eph,
		  { "Position Accuracy", "isi.gps.eph", FT_FLOAT, BASE_NONE, NULL, 0x0, "EPH (position accuracy) in meter", HFILL }},
		{ &hf_isi_gps_altitude,
		  { "Altitude", "isi.gps.alt", FT_INT16, BASE_DEC, NULL, 0x0, "Altitude in meter", HFILL }},
		{ &hf_isi_gps_epv,
		  { "Altitude Accuracy", "isi.gps.epv", FT_FLOAT, BASE_NONE, NULL, 0x0, "EPV (altitude accuracy) in meter", HFILL }},
		{ &hf_isi_gps_course,
		  { "Course", "isi.gps.course", FT_FLOAT, BASE_NONE, NULL, 0x0, "Course in degree", HFILL }},
		{ &hf_isi_gps_epd,
		  { "Course Accuracy", "isi.gps.epd", FT_FLOAT, BASE_NONE, NULL, 0x0, "EPD (course accuracy) in degree", HFILL }},
		{ &hf_isi_gps_speed,
		  { "Speed", "isi.gps.speed", FT_FLOAT, BASE_NONE, NULL, 0x0, "Speed in km/h", HFILL }},
		{ &hf_isi_gps_eps,
		  { "Speed Accuracy", "isi.gps.eps", FT_FLOAT, BASE_NONE, NULL, 0x0, "EPS (speed accuracy) in km/h", HFILL }},
		{ &hf_isi_gps_climb,
		  { "Climb", "isi.gps.climb", FT_FLOAT, BASE_NONE, NULL, 0x0, "Climb in km/h", HFILL }},
		{ &hf_isi_gps_satellites,
		  { "Visible Satellites", "isi.gps.satellites", FT_UINT8, BASE_DEC, NULL, 0x0, "Visible Satellites", HFILL }},
		{ &hf_isi_gps_prn,
		  { "Pseudeorandom Noise (PRN)", "isi.gps.sat.prn", FT_UINT8, BASE_HEX_DEC, NULL, 0x0, "Pseudeorandom Noise (PRN)", HFILL }},
		{ &hf_isi_gps_sat_used,
		  { "in use", "isi.gps.sat.used", FT_BOOLEAN, BASE_NONE, NULL, 0x0, "in use", HFILL }},
		{ &hf_isi_gps_sat_strength,
		  { "Signal Strength", "isi.gps.sat.strength", FT_FLOAT, BASE_NONE, NULL, 0x0, "Signal Strength", HFILL }},
		{ &hf_isi_gps_sat_elevation,
		  { "Elevation", "isi.gps.sat.elevation", FT_FLOAT, BASE_NONE, NULL, 0x0, "Elevation", HFILL }},
		{ &hf_isi_gps_sat_azimuth,
		  { "Azimuth", "isi.gps.sat.azimuth", FT_FLOAT, BASE_NONE, NULL, 0x0, "Azimuth", HFILL }},
		{ &hf_isi_gps_epc,
		  { "Climb Accuracy", "isi.gps.epc", FT_FLOAT, BASE_NONE, NULL, 0x0, "EPC (climb accuracy) in km/h", HFILL }},
		{ &hf_isi_gps_mcc,
		  { "Mobile Country Code (MCC)", "isi.gps.gsm.mcc", FT_UINT16, BASE_HEX_DEC, NULL, 0x0, "Mobile Country Code (MCC)", HFILL }},
		{ &hf_isi_gps_mnc,
		  { "Mobile Network Code (MNC)", "isi.gps.gsm.mnc", FT_UINT16, BASE_HEX_DEC, NULL, 0x0, "Mobile Network Code (MNC)", HFILL }},
		{ &hf_isi_gps_lac,
		  { "Location Area Code (LAC)", "isi.gps.gsm.lac", FT_UINT16, BASE_HEX_DEC, NULL, 0x0, "Location Area Code (LAC)", HFILL }},
		{ &hf_isi_gps_cid,
		  { "Cell ID (CID)", "isi.gps.gsm.cid", FT_UINT16, BASE_HEX_DEC, NULL, 0x0, "Cell ID (CID)", HFILL }},
		{ &hf_isi_gps_ucid,
		  { "Cell ID (UCID)", "isi.gps.gsm.ucid", FT_UINT32, BASE_HEX_DEC, NULL, 0x0, "Cell ID (UCID)", HFILL }}
	};

	proto_register_field_array(proto_isi, hf, array_length(hf));
	register_dissector("isi.gps", dissect_isi_gps, proto_isi);
}

static void dissect_isi_gps_data(tvbuff_t *tvb, packet_info *pinfo, proto_item *item, proto_tree *tree) {
	guint8 len = tvb->length;
	int i;

	guint8 pkgcount = tvb_get_guint8(tvb, 0x07);
	proto_tree_add_item(tree, hf_isi_gps_sub_pkgs, tvb, 0x07, 1, FALSE);

	size_t offset = 0x0b; // subpackets start here
	for(i=0; i<pkgcount; i++) {
		guint8 sptype = tvb_get_guint8(tvb, offset+1);
		guint8 splen = tvb_get_guint8(tvb, offset+3);

		proto_item *subitem = proto_tree_add_text(tree, tvb, offset, splen, "Subpacket (%s)", val_to_str(sptype, isi_gps_sub_id, "unknown: 0x%x"));
		proto_tree *subtree = proto_item_add_subtree(subitem, ett_isi_msg);

		proto_tree_add_item(subtree, hf_isi_gps_sub_type, tvb, offset+1, 1, FALSE);
		proto_tree_add_item(subtree, hf_isi_gps_sub_len, tvb,  offset+3, 1, FALSE);

		offset += 4;
		switch(sptype) {
			case 0x02: ; // Position
				double lat = tvb_get_ntohl(tvb, offset+0);
				lat = (lat*360)/4294967296;
				if(lat > 180.0) lat -= 360.0;
				proto_tree_add_double(subtree, hf_isi_gps_latitude, tvb, offset+0, 4, lat);

				double lon = tvb_get_ntohl(tvb, offset+4);
				lon = (lon*360)/4294967296;
				if(lon > 180.0) lon -= 360.0;
				proto_tree_add_double(subtree, hf_isi_gps_longitude, tvb, offset+4, 4, lon);

				float eph = tvb_get_ntohl(tvb, offset+12) / 100.0;
				proto_tree_add_float(subtree, hf_isi_gps_eph, tvb, offset+12, 4, eph);

				gint32 altitude = (tvb_get_ntohs(tvb, offset+18) - tvb_get_ntohs(tvb, offset+22))/2;
				proto_tree_add_int(subtree, hf_isi_gps_altitude, tvb, offset+18, 6, altitude);

				float epv = tvb_get_ntohs(tvb, offset+20) / 2;
				proto_tree_add_float(subtree, hf_isi_gps_epv, tvb, offset+20, 2, epv);

				break;
			case 0x03: // Date and Time
				proto_tree_add_item(subtree, hf_isi_gps_year,    tvb, offset+0, 2, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_month,   tvb, offset+2, 1, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_day,     tvb, offset+3, 1, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_hour,    tvb, offset+5, 1, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_minute,  tvb, offset+6, 1, FALSE);

				float second = tvb_get_ntohs(tvb, offset+8) / 1000.0;
				proto_tree_add_float(subtree, hf_isi_gps_second, tvb, offset+8, 2, second);
				break;
			case 0x04: ; // Movement
				float course = tvb_get_ntohs(tvb, offset+0) / 100.0;
				proto_tree_add_float(subtree, hf_isi_gps_course, tvb, offset+0, 2, course);

				float epd = tvb_get_ntohs(tvb, offset+2) / 100.0;
				proto_tree_add_float(subtree, hf_isi_gps_epd, tvb, offset+2, 2, epd);

				float speed = tvb_get_ntohs(tvb, offset+6) * CMS_TO_KMH;
				proto_tree_add_float(subtree, hf_isi_gps_speed, tvb, offset+6, 2, speed);

				float eps = tvb_get_ntohs(tvb, offset+8) * CMS_TO_KMH;
				proto_tree_add_float(subtree, hf_isi_gps_eps, tvb, offset+8, 2, eps);

				float climb = tvb_get_ntohs(tvb, offset+10) * CMS_TO_KMH;
				proto_tree_add_float(subtree, hf_isi_gps_climb, tvb, offset+10, 2, climb);

				float epc = tvb_get_ntohs(tvb, offset+12) * CMS_TO_KMH;
				proto_tree_add_float(subtree, hf_isi_gps_epc, tvb, offset+12, 2, epc);
				break;
			case 0x05: ; // Satellite Info
				guint8 satellites = tvb_get_guint8(tvb, offset+0);
				proto_tree_add_item(subtree, hf_isi_gps_satellites, tvb, offset+0, 1, FALSE);
				
				int sat;
				for(sat = 0; sat < satellites ; sat++) {
					int pos = offset+4+(sat*SAT_PKG_LEN);
					proto_item *satitem = proto_tree_add_text(subtree, tvb, pos, SAT_PKG_LEN, "Satellite %d", sat);
					proto_tree *sattree = proto_item_add_subtree(satitem, ett_isi_msg);

					float signal_strength = tvb_get_ntohs(tvb, pos+3) / 100.0;
					float elevation       = tvb_get_ntohs(tvb, pos+6) / 100.0;
					float azimuth         = tvb_get_ntohs(tvb, pos+8) / 100.0;

					proto_tree_add_item(sattree, hf_isi_gps_prn,            tvb, pos+1, 1, FALSE);
					proto_tree_add_item(sattree, hf_isi_gps_sat_used,       tvb, pos+2, 1, FALSE);
					proto_tree_add_float(sattree, hf_isi_gps_sat_strength,  tvb, pos+3, 2, signal_strength);
					proto_tree_add_float(sattree, hf_isi_gps_sat_elevation, tvb, pos+6, 2, elevation);
					proto_tree_add_float(sattree, hf_isi_gps_sat_azimuth,   tvb, pos+8, 2, azimuth);
				}
				break;
			case 0x07: // CellInfo GSM
				proto_tree_add_item(subtree, hf_isi_gps_mcc,  tvb, offset+0, 2, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_mnc,  tvb, offset+2, 2, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_lac,  tvb, offset+4, 2, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_cid,  tvb, offset+6, 2, FALSE);
				break;
			case 0x08: // CellInfo WCDMA
				proto_tree_add_item(subtree, hf_isi_gps_mcc,  tvb, offset+0, 2, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_mnc,  tvb, offset+2, 2, FALSE);
				proto_tree_add_item(subtree, hf_isi_gps_ucid, tvb, offset+4, 4, FALSE);
				break;
			default:
				break;
		}

		offset += splen - 4;
	}

}

static void dissect_isi_gps(tvbuff_t *tvb, packet_info *pinfo, proto_item *isitree) {
	proto_item *item = NULL;
	proto_tree *tree = NULL;
	guint8 cmd;

	if(isitree) {
		item = proto_tree_add_text(isitree, tvb, 0, -1, "Payload");
		tree = proto_item_add_subtree(item, ett_isi_msg);

		proto_tree_add_item(tree, hf_isi_gps_cmd, tvb, 0, 1, FALSE);
		cmd = tvb_get_guint8(tvb, 0);

		switch(cmd) {
			case 0x7d: /* GPS Status */
				proto_tree_add_item(tree, hf_isi_gps_status, tvb, 2, 1, FALSE);
				guint8 status = tvb_get_guint8(tvb, 2);
				col_add_fstr(pinfo->cinfo, COL_INFO, "GPS Status Indication: %s", val_to_str(status, isi_gps_status, "unknown (0x%x)"));
				break;
			case 0x84:
			case 0x85:
			case 0x86:
			case 0x87:
			case 0x88:
			case 0x89:
			case 0x8a:
			case 0x8b:
				col_add_fstr(pinfo->cinfo, COL_INFO, "unknown A-GPS package (0x%02x)", cmd);
				break;
			case 0x90: /* GPS Power Request */
				col_set_str(pinfo->cinfo, COL_INFO, "GPS Power Request");
				break;
			case 0x91: /* GPS Power Request */
				col_set_str(pinfo->cinfo, COL_INFO, "GPS Power Response");
				break;
			case 0x92: /* GPS Data */
				col_set_str(pinfo->cinfo, COL_INFO, "GPS Data");
				dissect_isi_gps_data(tvb, pinfo, item, tree);
				break;
			default:
				col_add_fstr(pinfo->cinfo, COL_INFO, "unknown GPS package (0x%02x)", cmd);
				break;
		}
	}
}
