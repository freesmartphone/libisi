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

typedef enum {
	ISI_SIM_AUTH_STATUS_ERROR,
	ISI_SIM_AUTH_STATUS_NO_SIM,
	ISI_SIM_AUTH_STATUS_NEED_NONE,
	ISI_SIM_AUTH_STATUS_NEED_PIN,
	ISI_SIM_AUTH_STATUS_NEED_PUK,
	ISI_SIM_AUTH_STATUS_VALID_PIN,
	ISI_SIM_AUTH_STATUS_VALID_PUK,
	ISI_SIM_AUTH_STATUS_INVALID_PIN,
	ISI_SIM_AUTH_STATUS_INVALID_PUK,
	ISI_SIM_AUTH_STATUS_AUTHORIZED,
	ISI_SIM_AUTH_STATUS_INITIALIZING,
	ISI_SIM_AUTH_STATUS_PROTECTED,
	ISI_SIM_AUTH_STATUS_UNPROTECTED
} IsiSimAuthStatus;

typedef enum {
	ISI_SIM_AUTH_ANSWER_OK,
	ISI_SIM_AUTH_ANSWER_ERR_UNKNOWN,
	ISI_SIM_AUTH_ANSWER_ERR_PIN_TOO_LONG,
	ISI_SIM_AUTH_ANSWER_ERR_PUK_TOO_LONG,
	ISI_SIM_AUTH_ANSWER_ERR_INVALID,
	ISI_SIM_AUTH_ANSWER_ERR_NEED_PUK
} IsiSimAuthAnswer;

/* creation & destruction */
struct isi_sim_auth* isi_sim_auth_create(struct isi_modem *modem);
void isi_sim_auth_destroy(struct isi_sim_auth *nd);

/* callbacks */
typedef void (*isi_sim_auth_cb)(IsiSimAuthAnswer code, void *user_data);
typedef void (*isi_sim_auth_status_cb)(IsiSimAuthStatus code, void *user_data);

/* methods */
void isi_sim_auth_set_pin(struct isi_sim_auth *nd, char *pin, isi_sim_auth_cb cb, void *user_data);
void isi_sim_auth_set_puk(struct isi_sim_auth *nd, char *puk, char *pin, isi_sim_auth_cb cb, void *user_data);
void isi_sim_update_pin(struct isi_sim_auth *nd, char *old_pin, char *new_pin, isi_sim_auth_cb cb, void *user_data);

void isi_sim_auth_get_pin_protection(struct isi_sim_auth *nd, isi_sim_auth_status_cb cb, void *user_data);
void isi_sim_auth_set_pin_protection(struct isi_sim_auth *nd, char *pin, gboolean status, isi_sim_auth_status_cb cb, void *user_data);

void isi_sim_auth_request_status(struct isi_sim_auth *nd, isi_sim_auth_status_cb cb, void *user_data);
void isi_sim_auth_subscribe_status(struct isi_sim_auth *nd, isi_sim_auth_status_cb cb, void *user_data);
void isi_sim_auth_unsubscribe_status(struct isi_sim_auth *nd);


#endif
