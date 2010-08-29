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

enum isi_sim_auth_status {
	SIM_AUTH_STATUS_ERROR,
	SIM_AUTH_STATUS_NO_SIM,
	SIM_AUTH_STATUS_NEED_NONE,
	SIM_AUTH_STATUS_NEED_PIN,
	SIM_AUTH_STATUS_NEED_PUK,
	SIM_AUTH_STATUS_VALID_PIN,
	SIM_AUTH_STATUS_VALID_PUK,
	SIM_AUTH_STATUS_INVALID_PIN,
	SIM_AUTH_STATUS_INVALID_PUK,
	SIM_AUTH_STATUS_AUTHORIZED
};

enum isi_sim_auth_answer {
	SIM_AUTH_OK,
	SIM_AUTH_ERR_UNKNOWN,
	SIM_AUTH_ERR_PIN_TOO_LONG,
	SIM_AUTH_ERR_PUK_TOO_LONG,
	SIM_AUTH_ERR_INVALID,
	SIM_AUTH_ERR_NEED_PUK
};

/* creation & destruction */
struct isi_sim_auth* isi_sim_auth_create(struct isi_modem *modem);
void isi_sim_auth_destroy(struct isi_sim_auth *nd);

/* callbacks */
typedef void (*isi_sim_auth_cb)(enum isi_sim_auth_answer code, void *user_data);
typedef void (*isi_sim_auth_status_cb)(enum isi_sim_auth_status code, void *user_data);

/* methods */
void isi_sim_auth_set_pin(struct isi_sim_auth *nd, char *pin, isi_sim_auth_cb cb, void *user_data);
void isi_sim_auth_set_puk(struct isi_sim_auth *nd, char *puk, char *pin, isi_sim_auth_cb cb, void *user_data);
void isi_sim_auth_request_status(struct isi_sim_auth *nd, isi_sim_auth_status_cb cb, void *user_data);
void isi_sim_auth_subscribe_status(struct isi_sim_auth *nd, isi_sim_auth_status_cb cb, void *user_data);
void isi_sim_auth_unsubscribe_status(struct isi_sim_auth *nd);

#endif
