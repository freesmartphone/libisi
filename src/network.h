#include <glib.h>
#include <stdint.h>
#include "opcodes/network.h"
#include "gisi/client.h"
#include "modem.h"

#ifndef ISI_NETWORK_H
#define ISI_NETWORK_H

/* Theoretical limit is 16, but each GSM char can be encoded into
 *  * 3 UTF8 characters resulting in 16*3=48 chars
 *   */
#define ISI_MAX_OPERATOR_NAME_LENGTH 63

struct network_status {
	enum net_reg_status status;
	guint16 lac;
	guint16 cid;
	enum net_technology technology;
};

struct network_operator {
	char name[ISI_MAX_OPERATOR_NAME_LENGTH + 1];
	char mcc[4]; // 3 + \0
	char mnc[4]; // 2-3 + \0
	int status;
};

struct network_data {
	GIsiClient *client;
	guint8 last_reg_mode;
	guint8 rat;
	guint8 gsm_compact;
};

/* callbacks */
typedef void (*isi_network_status_cb)(bool error, struct network_status *status, void *data);
typedef void (*isi_network_strength_cb)(bool error, uint8_t strength, void *data);
typedef void (*isi_network_register_cb)(bool error, void *data);
typedef void (*isi_network_operator_cb)(bool error, struct network_operator *something, void *data);
typedef void (*isi_network_operator_list_cb)(bool error, int total, const struct network_operator *list, void *data);

/* subsystem */
struct network_data* isi_network_create(GIsiModem *idx, isi_subsystem_reachable_cb cb, void *data);
void isi_network_destroy(struct network_data *nd);

/* status */
void isi_network_request_status(struct network_data *nd, isi_network_status_cb cb, void *user_data);
void isi_network_subscribe_status(struct network_data *nd, isi_network_status_cb cb, void *user_data);
void isi_network_unsubscribe_status(struct network_data *nd);

/* strength */
void isi_network_request_strength(struct network_data *nd, isi_network_strength_cb cb, void *user_data);
void isi_network_subscribe_strength(struct network_data *nd, isi_network_strength_cb cb, void *user_data);
void isi_network_unsubscribe_strength(struct network_data *nd);

/* registering */
void isi_network_register_manual(struct network_data *nd, const char *mcc, const char *mnc, isi_network_register_cb cb, void *data);
void isi_network_register_auto(struct network_data *nd, isi_network_register_cb cb, void *data);
void isi_network_deregister(struct network_data *nd, isi_network_register_cb cb, void *data);

/* operator */
void isi_network_current_operator(struct network_data *nd, isi_network_operator_cb cb, void *data);
void isi_network_list_operators(struct network_data *nd, isi_network_operator_list_cb cb, void *data);

#endif
