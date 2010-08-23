#include <glib.h>
#include <stdint.h>
#include "opcodes/sim.h"
#include "gisi/client.h"
#include "modem.h"

#ifndef _ISI_DEVICE_INFO_H
#define _ISI_DEVICE_INFO_H

struct isi_sim {
	GIsiClient *client;
};

/* callbacks */
#if 0
typedef void (*isi_device_info_cb)(gboolean error, const char *msg, void *user_data);
#endif

/* subsystem */
struct isi_sim* isi_sim_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data);
void isi_sim_destroy(struct isi_sim *nd);

#if 0
void isi_device_info_query_manufacturer(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);
void isi_device_info_query_model(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);
void isi_device_info_query_revision(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);
void isi_device_info_query_serial(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);
#endif

#endif
