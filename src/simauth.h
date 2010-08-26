#include <glib.h>
#include <stdint.h>
#include "opcodes/simauth.h"
#include "gisi/client.h"
#include "modem.h"

#ifndef _ISI_SIM_AUTH_H
#define _ISI_SIM_AUTH_H

struct isi_sim_auth {
	GIsiClient *client;
};

enum isi_sim_auth_answer {
	SIM_AUTH_OK,
	SIM_AUTH_UNKNOWN_ERROR,
	SIM_AUTH_TIMEOUT_ERROR,
	SIM_AUTH_PIN_TOO_LONG,
	SIM_AUTH_PUK_TOO_LONG,
	SIM_AUTH_PIN_INVALID,
	SIM_AUTH_PUK_INVALID,
	SIM_AUTH_NEED_PUK
};

/* creation & destruction */
struct isi_sim_auth* isi_sim_auth_create(struct isi_modem *modem);
void isi_sim_auth_destroy(struct isi_sim_auth *nd);

/* callbacks */
typedef void (*isi_sim_auth_cb)(enum isi_sim_auth_answer code, void *user_data);

/* methods */
void isi_sim_auth_set_pin(struct isi_sim_auth *nd, char *pin, isi_sim_auth_cb cb, void *user_data);
void isi_sim_auth_set_puk(struct isi_sim_auth *nd, char *puk, char *pin, isi_sim_auth_cb cb, void *user_data);

#endif
