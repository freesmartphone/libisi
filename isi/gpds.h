#include <glib.h>
#include <stdint.h>
#include "opcodes/gpds.h"
#include "gisi/client.h"
#include "modem.h"

#ifndef _ISI_GPDS_H
#define _ISI_GPDS_H

struct isi_gpds {
	GIsiClient *client;
};

struct isi_gpds* isi_gpds_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data);
void isi_gpds_destroy(struct isi_gpds *nd);

#endif
