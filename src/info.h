#include <glib.h>
#include <stdint.h>
#include "opcodes/info.h"
#include "gisi/client.h"
#include "modem.h"

#ifndef _ISI_DEVICE_INFO_H
#define _ISI_DEVICE_INFO_H

struct isi_device_info {
	GIsiClient *client;
};

/* callbacks */
typedef void (*isi_device_info_cb)(gboolean error, char *msg, char *user_data);

/* subsystem */
struct isi_device_info* isi_device_info_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data);
void isi_device_info_destroy(struct isi_device_info *nd);

void isi_device_info_query_manufacturer(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);
void isi_device_info_query_model(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);
void isi_device_info_query_revision(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);
void isi_device_info_query_serial(struct isi_device_info *nd, isi_device_info_cb cb, void *user_data);

#endif
