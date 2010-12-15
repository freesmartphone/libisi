#include "gisi/client.h"
#include "gisi/netlink.h"

#ifndef _ISI_MODEM_H
#define _ISI_MODEM_H

typedef void (*isi_subsystem_reachable_cb)(gboolean error, void *data);
typedef void (*isi_powerstatus_cb)(gboolean power, void *data);

struct isi_modem {
	GIsiModem *idx;
	GIsiClient *client;
	GPhonetNetlink *link;
	gboolean status;
	gboolean power;
	isi_powerstatus_cb powerstatus;
	void *user_data;
};

struct isi_modem* isi_modem_create(char *interface, isi_subsystem_reachable_cb cb, void *user_data);
void isi_modem_set_powerstatus_notification(struct isi_modem *modem, isi_powerstatus_cb cb, void *user_data);
gboolean isi_modem_get_powerstatus(struct isi_modem *modem);
void isi_modem_destroy(struct isi_modem *modem);
int isi_modem_enable(struct isi_modem *modem);
int isi_modem_disable(struct isi_modem *modem);

#endif
