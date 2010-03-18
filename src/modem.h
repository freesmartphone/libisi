#include "gisi/client.h"
#include "gisi/netlink.h"

#ifndef ISI_MODEM_H
#define ISI_MODEM_H

typedef void (*isi_subsystem_reachable_cb)(bool error, void *data);
typedef void (*isi_powerstatus_cb)(bool power, void *data);

struct isi_modem {
	GIsiModem *idx;
	GIsiClient *client;
	GPhonetNetlink *link;
	bool status;
	bool power;
	isi_powerstatus_cb powerstatus;
	void *user_data;
};

struct isi_modem* isi_modem_create(char *interface, isi_subsystem_reachable_cb cb, void *user_data);
void isi_modem_set_powerstatus_notification(struct isi_modem *modem, isi_powerstatus_cb cb, void *user_data);
bool isi_modem_get_powerstatus(struct isi_modem *modem);
void isi_modem_destroy(struct isi_modem *modem);
int isi_modem_enable(struct isi_modem *modem);
int isi_modem_disable(struct isi_modem *modem);

#endif
