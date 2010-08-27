#include <glib.h>
#include <stdint.h>
#include "opcodes/sim.h"
#include "gisi/client.h"
#include "modem.h"

#ifndef _ISI_SIM_H
#define _ISI_SIM_H

struct isi_sim {
	GIsiClient *client;
};

/* sim */
struct isi_sim* isi_sim_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data);
void isi_sim_destroy(struct isi_sim *nd);

#endif
