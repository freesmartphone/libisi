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

static const value_string isi_gps_id[] = {
	//{0x0d, "GPS_UNKNOWN_0D"},
	//{0x0e, "GPS_UNKNOWN_0E"},
	{0x7d, "GPS_STATUS"},
	//{0x7e, "GPS_UNKNOWN_7E"},
	//{0x7f, "GPS_UNKNOWN_7F"},
	//{0x82, "GPS_UNKNOWN_82"},
	//{0x83, "GPS_UNKNOWN_83"},
	//{0x84, "GPS_UNKNOWN_84"},
	//{0x85, "GPS_UNKNOWN_85"},
	//{0x90, "GPS_UNKNOWN_90"},
	//{0x91, "GPS_UNKNOWN_91"},
	{0x92, "GPS_DATA"},
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
		{ &hf_isi_gps_epc,
		  { "Climb Accuracy", "isi.gps.epc", FT_FLOAT, BASE_NONE, NULL, 0x0, "EPC (climb accuracy) in km/h", HFILL }}
	};

	proto_register_field_array(proto_isi, hf, array_length(hf));
	register_dissector("isi.gps", dissect_isi_gps, proto_isi);
}

static void dissect_isi_gps(tvbuff_t *tvb, packet_info *pinfo, proto_item *isitree) {
	proto_item *item = NULL;
	proto_tree *tree = NULL;
	guint8 cmd, code;
	guint8 len = tvb->length;

	if(isitree) {
		item = proto_tree_add_text(isitree, tvb, 0, -1, "Message");
		tree = proto_item_add_subtree(item, ett_isi_msg);

		proto_tree_add_item(tree, hf_isi_gps_cmd, tvb, 0, 1, FALSE);
		cmd = tvb_get_guint8(tvb, 0);

		switch(cmd) {
			case 0x7d: /* GPS Status */
				proto_tree_add_item(tree, hf_isi_gps_status, tvb, 2, 1, FALSE);
				guint8 status = tvb_get_guint8(tvb, 2);
				col_add_fstr(pinfo->cinfo, COL_INFO, "GPS Status: %s", val_to_str(status, isi_gps_status, "unknown (0x%x)"));
				break;
			case 0x92: /* GPS Data */
				col_set_str(pinfo->cinfo, COL_INFO, "GPS Data");

				if(len > 0x18) { /* time & date */
					proto_item *dateitem = proto_tree_add_text(tree, tvb, 0x0f, 12, "Date/Time");
					proto_tree *datetree = proto_item_add_subtree(dateitem, ett_isi_msg);

					proto_tree_add_item(datetree, hf_isi_gps_year, tvb, 0x0f, 2, FALSE);
					proto_tree_add_item(datetree, hf_isi_gps_month, tvb, 0x11, 1, FALSE);
					proto_tree_add_item(datetree, hf_isi_gps_day, tvb, 0x12, 1, FALSE);
					proto_tree_add_item(datetree, hf_isi_gps_hour, tvb, 0x14, 1, FALSE);
					proto_tree_add_item(datetree, hf_isi_gps_minute, tvb, 0x15, 1, FALSE);
					float second = tvb_get_ntohs(tvb, 0x17) / 1000.0;
					proto_tree_add_float(datetree, hf_isi_gps_second, tvb, 0x17, 2, second);
				}

				if(len > 0x34) { /* position */
					proto_item *positem = proto_tree_add_text(tree, tvb, 0x1f, 24, "Position");
					proto_tree *postree = proto_item_add_subtree(positem, ett_isi_msg);

					double lat = tvb_get_ntohl(tvb, 0x1f);
					double lon = tvb_get_ntohl(tvb, 0x23);
					proto_tree_add_double(postree, hf_isi_gps_latitude, tvb,  0x1f, 4, (lat*360)/4294967296);
					proto_tree_add_double(postree, hf_isi_gps_longitude, tvb, 0x23, 4, (lon*360)/4294967296);

					float eph = tvb_get_ntohs(tvb, 0x2d) * CMS_TO_KMH;
					proto_tree_add_float(postree, hf_isi_gps_eph, tvb, 0x2d, 2, eph);

					gint32 altitude = (tvb_get_ntohs(tvb, 0x31) - tvb_get_ntohs(tvb, 0x35))/2;
					proto_tree_add_int(postree, hf_isi_gps_altitude, tvb, 0x31, 6, altitude);

					float epv = tvb_get_ntohs(tvb, 0x33) * CMS_TO_KMH / 2;
					proto_tree_add_float(postree, hf_isi_gps_epv, tvb, 0x33, 2, epv);
				}

				if(len > 0x48) { /* movement */
					proto_item *movitem = proto_tree_add_text(tree, tvb, 0x3b, 14, "Movement");
					proto_tree *movtree = proto_item_add_subtree(movitem, ett_isi_msg);

					float course = tvb_get_ntohs(tvb, 0x3b) / 100.0;
					proto_tree_add_float(movtree, hf_isi_gps_course, tvb, 0x3b, 2, course);

					float epd = tvb_get_ntohs(tvb, 0x3d) / 100.0;
					proto_tree_add_float(movtree, hf_isi_gps_epd, tvb, 0x3d, 2, epd);

					float speed = tvb_get_ntohs(tvb, 0x41) * CMS_TO_KMH;
					proto_tree_add_float(movtree, hf_isi_gps_speed, tvb, 0x41, 2, speed);

					float eps = tvb_get_ntohs(tvb, 0x43) * CMS_TO_KMH;
					proto_tree_add_float(movtree, hf_isi_gps_eps, tvb, 0x43, 2, eps);

					float climb = tvb_get_ntohs(tvb, 0x45) * CMS_TO_KMH;
					proto_tree_add_float(movtree, hf_isi_gps_climb, tvb, 0x45, 2, climb);

					float epc = tvb_get_ntohs(tvb, 0x47) * CMS_TO_KMH;
					proto_tree_add_float(movtree, hf_isi_gps_epc, tvb, 0x47, 2, epc);
				}
				break;
			default:
				col_add_fstr(pinfo->cinfo, COL_INFO, "unknown GPS package (0x%02x)", cmd);
				break;
		}
	}
}
