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

struct isi_sim_auth {
	GIsiClient *client;
};

enum isi_sim_pin_answer {
	SIM_PIN_UNKNOWN_ERROR = 0x00,
	SIM_PIN_OK = 0x01,
	SIM_PIN_TOO_LONG = 0x02,
	SIM_PIN_INVALID = 0x03
};

/* sim */
struct isi_sim* isi_sim_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data);
void isi_sim_destroy(struct isi_sim *nd);

/* sim auth */
typedef void (*isi_sim_pin_cb)(gboolean error, enum isi_sim_pin_answer code, void *user_data);

struct isi_sim_auth* isi_sim_auth_create(struct isi_modem *modem, isi_subsystem_reachable_cb cb, void *data);
void isi_sim_auth_destroy(struct isi_sim_auth *nd);

void isi_sim_auth_set_pin(struct isi_sim_auth *nd, char *pin, isi_sim_pin_cb cb, void *user_data);

#endif
