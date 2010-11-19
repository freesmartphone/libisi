#include <glib.h>
#include <stdint.h>
#include "opcodes/gps.h"
#include "gisi/client.h"
#include "modem.h"

#ifndef _ISI_GPS_H
#define _ISI_GPS_H

struct isi_gps {
	GIsiClient *client;
};


typedef enum {
	ISI_GPS_STATUS_ERROR,
	ISI_GPS_STATUS_DISABLED,
	ISI_GPS_STATUS_NOT_LOCKED,
	ISI_GPS_STATUS_LOCKED
} IsiGpsStatus;

struct isi_gps_data {
	
};

/* callbacks */
typedef void (*isi_gps_data_cb)(gboolean error, struct isi_gps_data *data, void *user_data);
typedef void (*isi_gps_status_cb)(IsiGpsStatus *data, void *user_data);

/* subsystem */
struct isi_network* isi_network_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *user_data);
void isi_network_destroy(struct isi_network *nd);

/* methods */
void isi_gps_status_subscribe(struct isi_gps *nd, isi_gps_status_cb cb, void *user_data);
void isi_gps_status_unsubscribe(struct isi_gps *nd);
void isi_gps_data_subscribe(struct isi_gps *nd, isi_gps_data_cb cb, void *user_data);
void isi_gps_data_unsubscribe(struct isi_gps *nd);

#endif
